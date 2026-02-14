typedef struct	language_data		LANGUAGE_DATA;
typedef struct	word_data		WORD_DATA;
typedef struct	trans_data		TRANS_DATA;

struct language_data
{
    LANGUAGE_DATA *next;
    char *name;
    char *whoname;
    bool race[MAX_PC_RACE];
    TRANS_DATA *trans;
};

struct trans_data
{
    char key;
    TRANS_DATA *next;
    WORD_DATA *words;
};

struct word_data
{
    WORD_DATA *next;
    char *org_word;
    char *untrans_word;
};

LANGUAGE_DATA * lang_table;
void do_langs(CHAR_DATA * ch, char *argument);
LANGUAGE_DATA *lookup_lang(char langname[], LANGUAGE_DATA *ld);
LANGUAGE_DATA *update_char_language (int lang_number);
