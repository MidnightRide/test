#include "test_manager.h"

#include "memory/linear_allocator_tests.h"
#include "containers/hashtable_tests.h"

#include <core/logger.h>

int main()
{
    // Always initialise the test manager first.
    test_manager_init();
    
    // TODO(satvik): add test registrations here.
    linear_allocator_register_tests();
    hashtable_register_tests();
    
    MDEBUG("Starting tests...");
    
    // Execute tests.
    test_manager_run_tests();
    
    return 0;
}