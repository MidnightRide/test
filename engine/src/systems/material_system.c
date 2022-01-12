#include "material_system.h"

#include "core/logger.h"
#include "core/mstring.h"
#include "containers/hashtable.h"
#include "math/mmath.h"
#include "renderer/renderer_frontend.h"
#include "systems/texture_system.h"

// TODO: temp: resource system.
#include "platform/filesystem.h"
// End temp.

typedef struct material_system_state
{
    material_system_config config;
    
    material default_material;
    
    // Array of registered materials
    material *registered_materials;
    
    // Hashtable for material lookups.
    hashtable registered_material_table;
} material_system_state;

typedef struct material_reference
{
    u64 reference_count;
    u32 handle;
    b8 auto_release;
} material_reference;

static material_system_state *state_ptr = 0;

b8 create_default_material(material_system_state *state);
b8 load_material(material_config config, material *m);
void destroy_material(material *m);
b8 load_configuration_file(const char *path, material_config *out_config);

b8 material_system_initialise(u64 *memory_requirement, void *state, material_system_config config)
{
    if (config.max_material_count == 0)
    {
        MFATAL("material_system_initialise - config.max_material_count must be > 0.");
        return false;
    }
    
    // Block of memory will contain state strucutre, then block for array, then block for hashtable.
    u64 struct_requirement = sizeof(material_system_state);
    u64 array_requirement = sizeof(material) * config.max_material_count;
    u64 hashtable_requirement = sizeof(material_reference) * config.max_material_count;
    *memory_requirement = struct_requirement + array_requirement + hashtable_requirement;
    
    if (!state) return true;
    
    state_ptr = state;
    state_ptr->config = config;
    
    // The array block is after the state. Already allocated, so just set the pointer.
    void *array_block = state + struct_requirement;
    state_ptr->registered_materials = array_block;
    
    // Hashtable is after array.
    void *hashtable_block = array_block + array_requirement;
    
    // Create a hashtable for material lookups.
    hashtable_create(sizeof(material_reference), 
                     config.max_material_count, 
                     hashtable_block, 
                     false, 
                     &state_ptr->registered_material_table);
    
    // Fill the hashtable with invalid references to use as default;
    material_reference invalid_ref;
    invalid_ref.auto_release = false;
    invalid_ref.handle = INVALID_ID;
    invalid_ref.reference_count = 0;
    hashtable_fill(&state_ptr->registered_material_table, &invalid_ref);
    
    // Invalidate all entries in material array
    u32 count = state_ptr->config.max_material_count;
    for (u32 i = 0; i < count; ++i)
    {
        state_ptr->registered_materials[i].id = INVALID_ID;
        state_ptr->registered_materials[i].generation = INVALID_ID;
        state_ptr->registered_materials[i].internal_id = INVALID_ID;
    }
    
    if (!create_default_material(state_ptr))
    {
        MFATAL("material_system_initialise failed to create default material, booting.");
        return false;
    }
    
    return true;
}

void material_system_shutdown(void *state)
{
    material_system_state *s = (material_system_state *)state;
    if (s)
    {
        // Invalidate all materials in the array.
        u32 count = s->config.max_material_count;
        for (u32 i = 0; i < count; ++i)
        {
            if (s->registered_materials[i].id != INVALID_ID)
            {
                destroy_material(&s->registered_materials[i]);
            }
        }
        
        // Destroy the default material.
        destroy_material(&s->default_material);
    }
    
    state_ptr = 0;
}

material *material_system_acquire(const char *name)
{
    // Load the given material configuration from disk.
    material_config config;
    
    // Load file from disk.
    // TODO: Should be able to be located anywhere.
    char *format_str = "assets/materials/%s.%s";
    char full_file_path[512];
    
    // TODO: Try different extensions.
    string_format(full_file_path, format_str, name, "mmt");
    if (!load_configuration_file(full_file_path, &config))
    {
        MERROR("Failed to load material file: '%s'. Null pointer will be returned", full_file_path);
        return 0;
    }
    
    // Now acquire from loaded config.
    return material_system_acquire_from_config(config);
}

material *material_system_acquire_from_config(material_config config)
{
    // Return default material.
    if (strings_equali(config.name, DEFAULT_MATERIAL_NAME))
    {
        return &state_ptr->default_material;
    }
    
    material_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_material_table, config.name, &ref))
    {
        if (ref.reference_count == 0)
        {
            ref.auto_release = config.auto_release;
        }
        ref.reference_count++;
        if (ref.handle == INVALID_ID)
        {
            // No material exists here, find free index.
            u32 count = state_ptr->config.max_material_count;
            material *m = 0;
            for (u32 i = 0; i < count; ++i)
            {
                if (state_ptr->registered_materials[i].id == INVALID_ID)
                {
                    // Free slot.
                    ref.handle = i;
                    m = &state_ptr->registered_materials[i];
                    break;
                }
            }
            
            // Make sure an empty slot was actually found.
            if (!m || ref.handle == INVALID_ID)
            {
                MFATAL("material_system_acquire - Material system cannot hold anymroe materials. Adjust config to allow for more.");
                return 0;
            }
            
            // Create new material
            if (!load_material(config, m))
            {
                MERROR("Failed to load material: '%s'", config.name);
                return 0;
            }
            
            if (m->generation == INVALID_ID)
            {
                m->generation = 0;
            }
            else
            {
                m->generation++;
            }
            
            // Also use the handle as the material id.
            m->id = ref.handle;
            MTRACE("Material '%s' does not yet exist. Created, and ref_count is now %i.", config.name, ref.reference_count);
        }
        else
        {
            MTRACE("Material '%s' already exists. ref_count increased to %i.", config.name, ref.reference_count);
        }
        
        // Update the entry
        hashtable_set(&state_ptr->registered_material_table, config.name, &ref);
        return &state_ptr->registered_materials[ref.handle];
    }
    
    // NOTE(satvik): This would only happen in the event something went wrong with the state.
    MERROR("material_system_state_acquire_from_config failed to acquire material '%s'. Null pointer will be returned.", 
           config.name);
    return 0;
}

void material_system_release(const char *name)
{
    // Ignore release requests for the default material.
    if (strings_equali(name, DEFAULT_MATERIAL_NAME))
    {
        return;
    }
    material_reference ref;
    if (state_ptr && hashtable_get(&state_ptr->registered_material_table, name, &ref))
    {
        if (ref.reference_count == 0)
        {
            MWARN("Tried to release nonexistent material: '%s'", name);
        }
        if (--ref.reference_count == 0 && ref.auto_release)
        {
            material *m = &state_ptr->registered_materials[ref.handle];
            
            // Destroy material
            destroy_material(m);
            
            // Reset the reference.
            ref.handle = INVALID_ID;
            ref.auto_release = false;
            MTRACE("Released material '%s'. Material unloaded because reference count=0 and auto_release=true.", name);
        }
        else
        {
            MTRACE("Released material '%s', now has a reference count of '%i' (auto_release=%s).", 
                   name, 
                   ref.reference_count, 
                   ref.auto_release ? "true" : "false");
        }
        
        // Update the entry.
        hashtable_set(&state_ptr->registered_material_table, name, &ref);
    }
    else
    {
        MERROR("material_system_release failed to release material '%s'.", name);
    }
}

material *material_system_get_default()
{
    if (state_ptr)
    {
        return &state_ptr->default_material;
    }
    
    MFATAL("material_system_get_default called before system is initialised.");
    return 0;
}

b8 load_material(material_config config, material *m)
{
    mzero_memory(m, sizeof(material));
    
    // Name
    string_ncopy(m->name, config.name, MATERIAL_NAME_MAX_LENGTH);
    
    // Diffuse colour
    m->diffuse_colour = config.diffuse_colour;
    
    // Diffuse map
    if (string_length(config.diffuse_map_name) > 0)
    {
        m->diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
        m->diffuse_map.texture = texture_system_acquire(config.diffuse_map_name, true);
        if (!m->diffuse_map.texture)
        {
            MWARN("Unable to load texture: '%s' for material '%s', using default.", config.diffuse_map_name, m->name);
            m->diffuse_map.texture = texture_system_get_default_texture();
        }
    }
    else
    {
        // NOTE(satvik): Only set for clarity, as call to mzero_memory above does this already.
        m->diffuse_map.use = TEXTURE_USE_UNKNOWN;
        m->diffuse_map.texture = 0;
    }
    
    // TODO: other maps
    
    // Send it off to the renderer to acquire resources.
    if (!renderer_create_material(m))
    {
        MERROR("Failed to acquire renderer resources for material '%s'.", m->name);
        return false;
    }
    
    return true;
}

void destroy_material(material *m)
{
    MTRACE("Destroying material '%s'...", m->name);
    
    // Release texture references.
    if (m->diffuse_map.texture)
    {
        texture_system_release(m->diffuse_map.texture->name);
    }
    
    // Release renderer resources.
    renderer_destroy_material(m);
    
    // Zero it out, invalidate ids.
    mzero_memory(m, sizeof(material));
    m->id = INVALID_ID;
    m->generation = INVALID_ID;
    m->internal_id = INVALID_ID;
}

b8 create_default_material(material_system_state *state)
{
    mzero_memory(&state->default_material, sizeof(material));
    state->default_material.id = INVALID_ID;
    state->default_material.generation = INVALID_ID;
    string_ncopy(state->default_material.name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    state->default_material.diffuse_colour = vec4_one();
    state->default_material.diffuse_map.use = TEXTURE_USE_MAP_DIFFUSE;
    state->default_material.diffuse_map.texture = texture_system_get_default_texture();
    
    if (!renderer_create_material(&state->default_material))
    {
        MFATAL("Failed to acquire renderer resources for default texture. Application cannot continue.");
    }
    
    return true;
}

b8 load_configuration_file(const char *path, material_config *out_config)
{
    file_handle f;
    if (!filesystem_open(path, FILE_MODE_READ, false, &f))
    {
        MERROR("load_configuration_file - unable to open file for reading: '%s'.", path);
        return false;
    }
    
    // Read each line of the file.
    char line_buf[512] = "";
    char *p = &line_buf[0];
    u64 line_length = 0;
    u32 line_number = 1;
    while(filesystem_read_line(&f, 511, &p, &line_length))
    {
        // Trim the string.
        char *trimmed = string_trim(line_buf);
        
        // Get the trimmed length.
        line_length = string_length(trimmed);
        
        // Skip blank lines and comments.
        if (line_length < 1 || trimmed[0] == '#')
        {
            line_number++;
            continue;
        }
        
        // Split line into var/value
        i32 equal_index = string_index_of(trimmed, '=');
        if (equal_index == -1)
        {
            MWARN("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", path, line_number);
            line_number++;
            continue;
        }
        
        // Assume a max of 64 characters for the variable name.
        char raw_var_name[64];
        mzero_memory(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char *trimmed_var_name = string_trim(raw_var_name); 
        
        char raw_value[446];
        mzero_memory(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1);
        char *trimmed_value = string_trim(raw_value); 
        
        // Process the variable.
        if (strings_equali(trimmed_var_name, "version"))
        {
            // TODO(satvik): version.
        }
        else if (strings_equali(trimmed_var_name, "name"))
        {
            string_ncopy(out_config->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_map_name"))
        {
            string_ncopy(out_config->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        }
        else if (strings_equali(trimmed_var_name, "diffuse_colour"))
        {
            // Parse the colour
            if (!string_to_vec4(trimmed_value, &out_config->diffuse_colour))
            {
                MWARN("Error parsing diffuse colour in file '%s'. Using default of white instead.", path);
                out_config->diffuse_colour = vec4_one();
            }
        }
        
        // Clear the line buffer.
        mzero_memory(line_buf, sizeof(char) * 512);
        line_number++;
    }
    
    filesystem_close(&f);
    
    return true;
}
