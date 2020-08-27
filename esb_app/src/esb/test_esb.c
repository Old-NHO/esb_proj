#include "munit.h"
#include "esb.h"

/**
 * If the name of a test function is "test_abc" then you should
 * define the setup and teardown functions by the names:
 * test_abc_setup and test_abc_tear_down respectively.
 */
static void *
test1_setup(const MunitParameter params[], void *user_data)
{
    /**
     * Return the data that will be used for test1. Here we
     * are just return a string. It can be a struct or anything.
     * The memory that you allocate here for the test data
     * has to be cleaned up in corresponding tear down function,
     * which in this case is test1_tear_down.
     */
    return strdup("/path/to/bmd.xml");
}

static void
test1_tear_down(void *fixture)
{
    /* Receives the pointer to the data if that that was created in
    test1_setup function. */
    free(fixture);
}

static MunitResult
test1(const MunitParameter params[], void *fixture)
{
    char *str = (char *)fixture;
    /**
     * Perform the checking of logic here as needed.
     * Typically, you will invoke the function under testing
     * here by passing it suitable data from fixture. Then,
     * use assertions to verify the expected results.
     * In this example, we are just checking the value of a
     * string which we were expecting to be available in the
     * fixture itself. This test will fail when you change the
     * string. You will need to recompile and re-run the tests
     * to see the effect of any changes in data in this example.
     */
    munit_assert_string_equal(str, "/path/to/bmd.xml");

    // Invoke the ESB function (or any other function to test)
    int status = process_esb_request(str);
    
    // Assert the expected results
    munit_assert_true(status == 0);
    return MUNIT_OK;
}

/* Define the setup and the test for test2 */
static void *
test2_setup(const MunitParameter params[], void *user_data)
{
    return strdup("TEST-2");
}

static void
test2_tear_down(void *fixture)
{
    free(fixture);
}

static MunitResult
test2(const MunitParameter params[], void *fixture)
{
    char *str = (char *)fixture;
    munit_assert_string_equal(str, "TEST-2");
    return MUNIT_OK;
}

/* Put all unit tests here. */
MunitTest esb_tests[] = {
    {
        "/my-test-1",   /* name */
        test1,  /* test function */
        test1_setup,    /* setup function for the test */
        test1_tear_down,    /* tear_down */
        MUNIT_TEST_OPTION_NONE, /* options */
        NULL                    /* parameters */
    },
    {
        "/my-test-2",   /* name */
        test2,  /* test function */
        test2_setup,    /* setup function for the test */
        test2_tear_down,    /* tear_down */
        MUNIT_TEST_OPTION_NONE, /* options */
        NULL                    /* parameters */
    },
    /* Mark the end of the array with an entry where the test
   * function is NULL */
    {NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}};

/* Arrange the test cases into a test suite. */
static const MunitSuite suite = {
  "/my-tests", /* name */
  esb_tests, /* tests */
  NULL, /* suites */
  1, /* iterations */
  MUNIT_SUITE_OPTION_NONE /* options */
};

/* Run the the test suite */
int main (int argc, const char* argv[]) {
  return munit_suite_main(&suite, NULL, argc, argv);
}