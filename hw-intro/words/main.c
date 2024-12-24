/*

  Word Count using dedicated lists

*/

/*
Copyright © 2019 University of California, Berkeley

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <assert.h>
#include <bits/getopt_core.h>
#include <errno.h>
#include <getopt.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#include "word_count.h"

/* Global data structure tracking the words encountered */
WordCount *word_counts = NULL;

/* The maximum length of each word in a file */
#define MAX_WORD_LEN 64

/*
 * 3.1.1 Total Word Count
 *
 * Returns the total amount of words found in infile.
 * Useful functions: fgetc(), isalpha().
 */
int num_words(FILE* infile) {
  int num_words = 0;

  int ch = fgetc(infile);
  while (ch >= 0) {
    if (isalpha(ch)) {
      int len = 0;
      while (ch >= 0 && isalpha(ch)) {
        ch = fgetc(infile);
        len++;
      }
      if (len > 1) {
        num_words++;
      }
    }

    ch = fgetc(infile);
  }

  if (ch != EOF) {
    return ch;
  }

  return num_words;
}

/*
 * 3.1.2 Word Frequency Count
 *
 * Given infile, extracts and adds each word in the FILE to `wclist`.
 * Useful functions: fgetc(), isalpha(), tolower(), add_word().
 * 
 * As mentioned in the spec, your code should not panic or
 * segfault on errors. Thus, this function should return
 * 1 in the event of any errors (e.g. wclist or infile is NULL)
 * and 0 otherwise.
 */
int count_words(WordCount **wclist, FILE *infile) {
  char *buff = malloc(sizeof(char) * MAX_WORD_LEN);

  for (int ch = fgetc(infile); ch != EOF; ch = fgetc(infile)) {
    int len = 0;
    while (ch != EOF && isalpha(ch)) {
      if (len == MAX_WORD_LEN) {
        return 1;
      }
      buff[len] = tolower(ch);
      len++;
      ch = fgetc(infile);
    }

    if (len > 1) {
      char *word = malloc(sizeof(char) * len + 1);
      if (word == NULL) {
        return 1;
      }
      strncpy(word, buff, len);
      word[len] = '\0';

      add_word(wclist, word);

      free(word);
    }
  }

  free(buff);
  return 0;
}

/*
 * Comparator to sort list by frequency.
 * Useful function: strcmp().
 */
static bool wordcount_less(const WordCount *wc1, const WordCount *wc2) {
  if (wc1->count < wc2->count) {
    return true;
  } else if (wc1->count > wc2->count) {
    return false;
  }

  int cmp = strcmp(wc1->word, wc2->word);
  if (cmp < 0) {
    return true;
  }

  return false;
}

// In trying times, displays a helpful message.
static int display_help(void) {
	printf("Flags:\n"
	    "--count (-c): Count the total amount of words in the file, or STDIN if a file is not specified. This is default behavior if no flag is specified.\n"
	    "--frequency (-f): Count the frequency of each word in the file, or STDIN if a file is not specified.\n"
	    "--help (-h): Displays this help message.\n");
	return 0;
}

/*
 * Handle command line flags and arguments.
 */
int main (int argc, char *argv[]) {

  // Count Mode (default): outputs the total amount of words counted
  bool count_mode = true;
  int total_words = 0;

  // Freq Mode: outputs the frequency of each word
  bool freq_mode = false;

  // Variables for command line argument parsing
  int i;
  static struct option long_options[] =
  {
      {"count", no_argument, 0, 'c'},
      {"frequency", no_argument, 0, 'f'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
  };

  // Sets flags
  while ((i = getopt_long(argc, argv, "cfh", long_options, NULL)) != -1) {
      switch (i) {
          case 'c':
              count_mode = true;
              freq_mode = false;
              break;
          case 'f':
              count_mode = false;
              freq_mode = true;
              break;
          case 'h':
              return display_help();
      }
  }

  if (!count_mode && !freq_mode) {
    printf("Please specify a mode.\n");
    return display_help();
  }

  /* Create the empty data structure */
  init_words(&word_counts);

  int file_count = (argc - optind) == 0 ? 1 : (argc - optind);
  FILE **infiles = malloc(sizeof(FILE **) * (file_count));

  if ((argc - optind) == 0) {
    infiles[0] = stdin;
  } else {
    // At least one file specified. Useful functions: fopen(), fclose().
    // The first file can be found at argv[optind]. The last file can be
    // found at argv[argc-1].
    for (int i = optind; i < argc; i++) {
      infiles[i - optind] = fopen(argv[i], "r");
      if (infiles[i - optind] == NULL) {
        fprintf(stderr, "error opening file %s\n", argv[i]);
        return errno;
      }
    }
  }

  if (count_mode) {
    for (int i = 0; i < file_count; i++) {
      total_words += num_words(infiles[i]);
    }
    printf("The total number of words is: %i\n", total_words);
  } else {
    for (int i = 0; i < file_count; i++) {
      count_words(&word_counts, infiles[i]);
    }

    wordcount_sort(&word_counts, wordcount_less);

    printf("The frequencies of each word are: \n");
    fprint_words(word_counts, stdout);
  }

  for (int i = 0; i < file_count; i++) {
    int err = fclose(infiles[i]);
    if (err != 0) {
      fprintf(stderr, "error when closing file");
      return err;
    }
  }

  return 0;
}
