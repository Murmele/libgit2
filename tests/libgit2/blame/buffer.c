#include "blame_helpers.h"

static git_repository *g_repo;
static git_blame *g_fileblame, *g_bufferblame;

void test_blame_buffer__initialize(void)
{
	cl_git_pass(git_repository_open(&g_repo, cl_fixture("blametest.git")));
	cl_git_pass(git_blame_file(&g_fileblame, g_repo, "b.txt", NULL));
	g_bufferblame = NULL;
}

void test_blame_buffer__cleanup(void)
{
	git_blame_free(g_fileblame);
	git_blame_free(g_bufferblame);
	git_repository_free(g_repo);
}

void test_blame_buffer__index(void)
{
	const git_blame_hunk *hunk;
	const char *buffer = "Hello\nWorld!";

	/*
	 * We need to open a different file from the ones used in other tests. Close
	 * the one opened in test_blame_buffer__initialize() to avoid a leak.
	 */
	git_blame_free(g_fileblame);
	g_fileblame = NULL;
	cl_git_pass(git_blame_file(&g_fileblame, g_repo, "file.txt", NULL));

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	cl_assert_equal_i(2, git_blame_get_hunk_count(g_bufferblame));

	check_blame_hunk_index(g_repo, g_bufferblame, 0,  1, 1, 0, "836bc00b", "file.txt");
	hunk = git_blame_get_hunk_byline(g_bufferblame, 1);
	cl_assert(hunk);
	cl_assert_equal_s("lhchavez", hunk->final_signature->name);
	check_blame_hunk_index(g_repo, g_bufferblame, 1,  2, 1, 0, "00000000", "file.txt");
	hunk = git_blame_get_hunk_byline(g_bufferblame, 2);
	cl_assert(hunk);
	cl_assert(hunk->final_signature == NULL);
}

void test_blame_buffer__added_line(void)
{
	const git_blame_hunk *hunk;

	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
abcdefg\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n";

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	cl_assert_equal_i(5, git_blame_get_hunk_count(g_bufferblame));
	check_blame_hunk_index(g_repo, g_bufferblame, 2, 6, 1, 0, "000000", "b.txt");

	hunk = git_blame_get_hunk_byline(g_bufferblame, 16);
	cl_assert(hunk);
	cl_assert_equal_s("Ben Straub", hunk->final_signature->name);
}

void test_blame_buffer__deleted_line(void)
{
	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n";

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	check_blame_hunk_index(g_repo, g_bufferblame, 2,  6, 3, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 3,  9, 1, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 4, 10, 5, 0, "aa06ecca", "b.txt");
}

void test_blame_buffer__add_splits_hunk(void)
{
	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
abc\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n";

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	check_blame_hunk_index(g_repo, g_bufferblame, 2, 6, 2, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 3, 8, 1, 0, "00000000", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 4, 9, 3, 0, "63d671eb", "b.txt");
}

void test_blame_buffer__delete_crosses_hunk_boundary(void)
{
	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n";

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	check_blame_hunk_index(g_repo, g_bufferblame, 2, 6, 1, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 3, 7, 2, 0, "aa06ecca", "b.txt");
}

void test_blame_buffer__replace_line(void)
{
	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
abc\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\n";

	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));
	check_blame_hunk_index(g_repo, g_bufferblame, 2, 6, 1, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 3, 7, 1, 0, "00000000", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 4, 8, 3, 0, "63d671eb", "b.txt");
}

void test_blame_buffer__add_lines_at_end(void)
{
	const char *buffer = "\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE\n\
\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB\n\
\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC\n\
\n\
abc\n\
def\n";
	cl_git_pass(git_blame_buffer(&g_bufferblame, g_fileblame, buffer, strlen(buffer)));

	cl_assert_equal_i(5, git_blame_get_hunk_count(g_bufferblame));
	check_blame_hunk_index(g_repo, g_bufferblame, 0,  1, 4, 0, "da237394", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 1,  5, 1, 1, "b99f7ac0", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 2,  6, 5, 0, "63d671eb", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 3, 11, 5, 0, "aa06ecca", "b.txt");
	check_blame_hunk_index(g_repo, g_bufferblame, 4, 16, 2, 0, "00000000", "b.txt");
}

int blame_progress(const git_oid *suspect, void *payload)
{
	static int counter = 0;

	counter ++;
	cl_assert_equal_i(3.1416, *((double*)payload));

	/* abort after the third suspect */
	if (counter >= 3)
		return -123;

	return 0;
}

/*!
 * \brief test_blame_buffer__abort_loading_blame
 * Abort loading blame after a few suspects
 */
void test_blame_buffer__abort_loading_blame(void)
{
	git_blame_options opts;
	git_blame_options_init(&opts, GIT_BLAME_OPTIONS_VERSION);

	double number = 3.1416;

	opts.progress_cb = &blame_progress;
	opts.payload = &number;

	cl_assert_equal_i(-123, git_blame_file(&g_fileblame, g_repo, "c.txt", &opts));
	g_bufferblame = NULL;
}

