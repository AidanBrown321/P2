#include <string.h>
#include "harness/unity.h"
#include "../src/lab.h"


void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}


void test_cmd_parse2(void)
{
     //The string we want to parse from the user.
     //foo -v
     char *stng = (char*)malloc(sizeof(char)*7);
     strcpy(stng, "foo -v");
     char **actual = cmd_parse(stng);
     //construct our expected output
     size_t n = sizeof(char*) * 6;
     char **expected = (char**) malloc(sizeof(char*) *6);
     memset(expected,0,n);
     expected[0] = (char*)malloc(sizeof(char)*4);
     expected[1] = (char*)malloc(sizeof(char)*3);
     expected[2] = (char*)NULL;

     strcpy(expected[0], "foo");
     strcpy(expected[1], "-v");
     TEST_ASSERT_EQUAL_STRING(expected[0],actual[0]);
     TEST_ASSERT_EQUAL_STRING(expected[1],actual[1]);
     TEST_ASSERT_FALSE(actual[2]);
     free(expected[0]);
     free(expected[1]);
     free(expected);
}

void test_cmd_parse(void)
{
     char **rval = cmd_parse("ls -a -l");
     TEST_ASSERT_TRUE(rval);
     TEST_ASSERT_EQUAL_STRING("ls", rval[0]);
     TEST_ASSERT_EQUAL_STRING("-a", rval[1]);
     TEST_ASSERT_EQUAL_STRING("-l", rval[2]);
     TEST_ASSERT_EQUAL_STRING(NULL, rval[3]);
     TEST_ASSERT_FALSE(rval[3]);
     cmd_free(rval);
}

void test_trim_white_no_whitespace(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "ls -a", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("ls -a", rval);
     free(line);
}

void test_trim_white_start_whitespace(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "  ls -a", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("ls -a", rval);
     free(line);
}

void test_trim_white_end_whitespace(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "ls -a  ", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("ls -a", rval);
     free(line);
}

void test_trim_white_both_whitespace_single(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, " ls -a ", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("ls -a", rval);
     free(line);
}

void test_trim_white_both_whitespace_double(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "  ls -a  ", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("ls -a", rval);
     free(line);
}

void test_trim_white_all_whitespace(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "  ", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("", rval);
     free(line);
}

void test_trim_white_mostly_whitespace(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "    a    ", 10);
     char *rval = trim_white(line);
     TEST_ASSERT_EQUAL_STRING("a", rval);
     free(line);
}

void test_get_prompt_default(void)
{
     char *prompt = get_prompt("MY_PROMPT");
     TEST_ASSERT_EQUAL_STRING(prompt, "shell>");
     free(prompt);
}

void test_get_prompt_custom(void)
{
     const char* prmpt = "MY_PROMPT";
     if(setenv(prmpt,"foo>",true)){
          TEST_FAIL();
     }

     char *prompt = get_prompt(prmpt);
     TEST_ASSERT_EQUAL_STRING(prompt, "foo>");
     free(prompt);
     unsetenv(prmpt);
}

void test_ch_dir_home(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "cd", 10);
     char **cmd = cmd_parse(line);
     char *expected = getenv("HOME");
     change_dir(cmd);
     char *actual = getcwd(NULL,0);
     TEST_ASSERT_EQUAL_STRING(expected, actual);
     free(line);
     free(actual);
     cmd_free(cmd);
}

void test_ch_dir_root(void)
{
     char *line = (char*) calloc(10, sizeof(char));
     strncpy(line, "cd /", 10);
     char **cmd = cmd_parse(line);
     change_dir(cmd);
     char *actual = getcwd(NULL,0);
     TEST_ASSERT_EQUAL_STRING("/", actual);
     free(line);
     free(actual);
     cmd_free(cmd);
}

// New Tests

/**
 * Test for empty command parsing
 */
 void test_cmd_parse_empty(void) {
     char **result = cmd_parse("");
     TEST_ASSERT_TRUE(result);
     TEST_ASSERT_NULL(result[0]);
     cmd_free(result);
     
     result = cmd_parse("   ");
     TEST_ASSERT_TRUE(result);
     TEST_ASSERT_NULL(result[0]);
     cmd_free(result);
 }



 /**
 * Test for built-in command detection
 */
void test_do_builtin_detection(void) {
     struct shell sh;
     sh_init(&sh);
     
     // Test exit command (can't fully test as it would exit the test process)
     char *exit_cmd[] = {"exit", NULL};
     TEST_ASSERT_TRUE(do_builtin(&sh, exit_cmd));
     
     // Test cd command
     char *cd_cmd[] = {"cd", NULL};
     TEST_ASSERT_TRUE(do_builtin(&sh, cd_cmd));
     
     // Test history command 
     char *history_cmd[] = {"history", NULL};
     TEST_ASSERT_TRUE(do_builtin(&sh, history_cmd));
     
     // Test non-builtin command
     char *ls_cmd[] = {"ls", NULL};
     TEST_ASSERT_FALSE(do_builtin(&sh, ls_cmd));
     
     sh_destroy(&sh);
 }

  /**
 * Test for command parsing with memory cleanup
 */
void test_cmd_free(void) {
     char **cmd = cmd_parse("test command");
     TEST_ASSERT_TRUE(cmd);
     cmd_free(cmd);
 }

 void test_parse_args_unknown_flag(void) {
     // Mimic argv with an unknown flag
     char *argv[] = {"myprogram", "-x", NULL};
     parse_args(2, argv);
     // Ensure no exit or crash
 }
 
 void test_sh_init_non_interactive(void) {
     struct shell sh;
     sh.shell_terminal = -1; // Simulate non-interactive
     sh_init(&sh);
     TEST_ASSERT_FALSE(sh.shell_is_interactive);
     sh_destroy(&sh);
 }
 
 void test_change_dir_invalid(void) {
     char *cd_cmd[] = {"cd", "/nonexistent", NULL};
     int result = change_dir(cd_cmd);
     TEST_ASSERT_EQUAL_INT(-1, result);
 }
 
 void test_cmd_parse_special_chars(void) {
     char **result = cmd_parse("echo $HOME");
     TEST_ASSERT_TRUE(result);
     TEST_ASSERT_EQUAL_STRING("echo", result[0]);
     TEST_ASSERT_EQUAL_STRING("$HOME", result[1]);
     TEST_ASSERT_NULL(result[2]);
     cmd_free(result);
 }


int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_cmd_parse);
  RUN_TEST(test_cmd_parse2);
  RUN_TEST(test_trim_white_no_whitespace);
  RUN_TEST(test_trim_white_start_whitespace);
  RUN_TEST(test_trim_white_end_whitespace);
  RUN_TEST(test_trim_white_both_whitespace_single);
  RUN_TEST(test_trim_white_both_whitespace_double);
  RUN_TEST(test_trim_white_all_whitespace);
  RUN_TEST(test_get_prompt_default);
  RUN_TEST(test_get_prompt_custom);
  RUN_TEST(test_ch_dir_home);
  RUN_TEST(test_ch_dir_root);
  RUN_TEST(test_cmd_parse_empty);
  RUN_TEST(test_cmd_free);
  RUN_TEST(test_do_builtin_detection);
  RUN_TEST(test_parse_args_unknown_flag);
  RUN_TEST(test_sh_init_non_interactive);
  RUN_TEST(test_change_dir_invalid);
  RUN_TEST(test_cmd_parse_special_chars);
  

  return UNITY_END();
}
