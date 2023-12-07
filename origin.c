#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/stat.h>


void init_gap_buff(line *lne);

// Used to move the gap left in the array
// Assuming that position belong to given array
void move_gap_left(line *lne, int position);

// Used to move the gap right in the array
void move_gap_right(line *lne, int position);

/* Move mouse cursor to particular position
 * RETURN:- line pointer with adjusting gap buffer, 
 * at that point data can be added or can be removed
 * NOTE:- position should belong to same block.
 */
line* move_cursor(line *subline, int position) ;

void init_colors() {        
        start_color();
        init_pair(ORANGE, ORANGE, COLOR_BLACK);
	init_pair(RED, RED, COLOR_BLACK);
        init_pair(GREEN, GREEN, COLOR_BLACK);
	init_pair(YELLOW, YELLOW, COLOR_BLACK);
	init_pair(WHITE, WHITE, COLOR_BLACK);
	init_pair(DBLUE, DBLUE, COLOR_BLACK);
	init_pair(LBLUE, LBLUE, COLOR_BLACK);
	init_pair(COMMENT, COMMENT, COLOR_BLACK);
        // 3(orange), 15(white), 9(red), 10(light_green), 11(yerllow), 14(light blue), 21(dark blue), 39(light blue)

}

TrieNode* init_keywords() {

        char grp1[][8] = {"int", "char", "double", "long", "auto",
                     "signed", "unsigned", "void", "float", "short"};
        char grp2[][8] = {"register", "extern", "static", "volatile", "const"};

        char grp3[][8] = {"typedef", "struct", "enum", "union", "scanf", "printf"};

        char grp4[][8] = {"continue", "break", "return", "sizeof", "include"};

        char grp5[][8] = {"for", "while", "do", "goto"};

        char grp6[][8] = {"if", "else", "switch", "case", "default"};

        TrieNode *root = getNode();

        // Construct trie
        int i;
        for (i = 0; i < ARRAY_SIZE(grp1); i++)
                insert(root, grp1[i], ORANGE);
        for (i = 0; i < ARRAY_SIZE(grp2); i++)
                insert(root, grp2[i], RED);
        for (i = 0; i < ARRAY_SIZE(grp3); i++)
                insert(root, grp3[i], GREEN);
        for (i = 0; i < ARRAY_SIZE(grp4); i++)
                insert(root, grp4[i], LBLUE);
        for (i = 0; i < ARRAY_SIZE(grp5); i++)
                insert(root, grp5[i], DBLUE);
        for (i = 0; i < ARRAY_SIZE(grp6); i++)
                insert(root, grp6[i], YELLOW);

        return root;
}

void init_window(win *w, int tot_lines) {
	w->head = (node_l*)malloc(sizeof(node_l) * tot_lines);
	w->tot_lines = tot_lines;
	w->head_indx = 0;

	for(int i = 0; i < tot_lines; i++) {
		// init subline with array of constant size
		(w->head)[i].line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
		// init next subline pointer with NULL
		(w->head)[i].line.rem_line = NULL;
		(w->head)[i].line_size = 0;
		init_gap_buff(&(w->head[i]).line);
	}
	return;
}


/*use to load text from file into ADT*/
FILE* load_file(win *w, char *filename) {
	FILE *fd_get = fopen(filename, "r");

	char c;
	line *lne, *empty_line;	
	for(int i = 0; i < w->tot_lines; i++) {
		lne = &((w->head)[i].line);
		int indx = 0;
		
		c = fgetc(fd_get);
        	// next line not present
        	if(c == -1) {
                	lne->curr_line[0] = MAX_CHAR;
                	return fd_get;
        	}
		ungetc(c, fd_get);

		while((c = fgetc(fd_get)) != '\n') {
			if(c == '\t')
				for(int i = 0; i < TAB_SPACE; i++)
					insert_at_pos(lne, indx++, ' ');
			else 
                		insert_at_pos(lne, indx++, c);
		}
		
		(w->head)[i].line_size = indx;	// size of total line
	}

	return fd_get;	
}

void init_gap_buff(line *lne) {
        // initially gap size = size of subline,
        // left will be at pos 0 and right at max-1
        lne->gap_left = 0;
        lne->gap_right = MAX_CHAR_IN_SUBLINE - 1;
        lne->gap_size = MAX_CHAR_IN_SUBLINE;
        return;
}


// Used to move the gap left in the array
// Assuming that position belong to given array
void move_gap_left(line *lne, int position) {
        char *buffer = lne->curr_line;
        int *gap_left = &(lne->gap_left);
        int *gap_right = &(lne->gap_right);

        if(lne->gap_size == 0) {
                *gap_left = position;   //TODO changed (don)
                *gap_right = position-1;
                return;
        }
        // Move the gap left character by character
        while (position < (*gap_left)) {
                (*gap_left)--;
                (*gap_right)--;
                buffer[(*gap_right) + 1] = buffer[*gap_left];
        }
        return;
}

// Used to move the gap right in the array
void move_gap_right(line *lne, int position) {
        // extract data from line structure to handle gap 
        char *buffer = lne->curr_line;
        int *gap_left = &(lne->gap_left);
        int *gap_right = &(lne->gap_right);

        // if gap size is zero
        if(lne->gap_size == 0) {
                if(position >= MAX_CHAR_IN_SUBLINE) {
                        *gap_left = MAX_CHAR_IN_SUBLINE;
                        *gap_right = MAX_CHAR_IN_SUBLINE - 1;
                        return;
                }
                *gap_left = position;
                *gap_right = position -1;
                return;
        }

        // Move the gap right character by character   
        while (position > (*gap_left)) {
                (*gap_left)++;
                (*gap_right)++;
                // For testing
                // TODO modify this
                if(*gap_right >= MAX_CHAR_IN_SUBLINE) {
                        (*gap_left)--;
                        (*gap_right)--;
                        //printf("already at last cannot move further right return from here\n");
                        return;
                }

                buffer[(*gap_left) - 1] = buffer[*gap_right];
        }
        return;
}

void init_window(win *w, int tot_lines) {
	w->head = (node_l*)malloc(sizeof(node_l) * tot_lines);
	w->tot_lines = tot_lines;
	w->head_indx = 0;

	for(int i = 0; i < tot_lines; i++) {
		// init subline with array of constant size
		(w->head)[i].line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
		// init next subline pointer with NULL
		(w->head)[i].line.rem_line = NULL;
		(w->head)[i].line_size = 0;
		init_gap_buff(&(w->head[i]).line);
	}
	return;
}


/*use to load text from file into ADT*/
FILE* load_file(win *w, char *filename) {
	FILE *fd_get = fopen(filename, "r");

	char c;
	line *lne, *empty_line;	
	for(int i = 0; i < w->tot_lines; i++) {
		lne = &((w->head)[i].line);
		int indx = 0;
		
		c = fgetc(fd_get);
        	// next line not present
        	if(c == -1) {
                	lne->curr_line[0] = MAX_CHAR;
                	return fd_get;
        	}
		ungetc(c, fd_get);

		while((c = fgetc(fd_get)) != '\n') {
			if(c == '\t')
				for(int i = 0; i < TAB_SPACE; i++)
					insert_at_pos(lne, indx++, ' ');
			else 
                		insert_at_pos(lne, indx++, c);
		}
		
		(w->head)[i].line_size = indx;	// size of total line
	}

	return fd_get;	
}

/* Move mouse cursor to particular position
 * RETURN:- line pointer with adjusting gap buffer, 
 * at that point data can be added or can be removed
 * NOTE:- position should belong to same block.
 */
line* move_cursor(line *subline, int position) {
	line *lne = subline;
	// take next subline till position doesn't belong to corresponding curr_line
	while((MAX_CHAR_IN_SUBLINE - lne->gap_size) < position) {
		// position not belong to current subline take next line
		// decrease position
		position -= (MAX_CHAR_IN_SUBLINE - lne->gap_size);
		// if current subline is last
		if(lne->rem_line == NULL)
			break;
		// take next subline
		lne = lne->rem_line;
	}
	
	// Now position belongs to current subline
	// move cursor(gap) at appropriate position
	if (position < lne->gap_left)
		move_gap_left(lne, position);
	else 
		move_gap_right(lne, position);
	return lne;
}

TrieNode_c* init_codebase(FILE *fd) {
	TrieNode_c *root = codebaseNode();
	char key[MAX_KEY];
	char ch;
	while(1) {
		if((ch = fgetc(fd)) == -1)
			return root;
		ungetc(ch, fd);

		fscanf(fd, "%s", key);
	
		int lower, upper;
		fscanf(fd, "%d", &lower);
		fscanf(fd, "%d", &upper);

		insert_in_codebase(root, key, lower, upper);
	}
}


char* accept_codebase_key(int x, int y) {
	move(CB_X, CB_Y);
	clrtoeol();
	refresh();
	char ch;
	char* arr = (char*)malloc(sizeof(char) * MAX_KEY);
	for(int i = 0; (ch = getch()) != '\n'; i++) {
		arr[i] = ch;
		mvprintw(CB_X, CB_Y + i, "%c", ch);
	}
	move(CB_X, CB_Y);
	clrtoeol();
	move(x, y);
	refresh();
	return arr;
}

void store_key(FILE *fd, char *key, int lower, int upper) {
	fprintf(fd, "%s\n", key);
	fprintf(fd, "%d\n", lower);
	fprintf(fd, "%d\n", upper);
	return;
}

void print_cbError() {
	move(CB_X, CB_Y);
	clrtoeol();
	refresh();
	mvprintw(CB_X, CB_Y, "%s", "Invalid Key, Press F1 to retry Or any other key to continue to editor...");
	refresh();
	return;
}

void print_cbAccept() {
        move(CB_X, CB_Y);
	clrtoeol();
        refresh();
        mvprintw(CB_X, CB_Y, "%s", "Now Select CODE and press 'CTRL+SHIFT+V' to store in CodeBase...");
        refresh();
        return;
}

void print_cbSuccess(char *key) {
	move(CB_X, CB_Y);
	clrtoeol();
        refresh();
        mvprintw(CB_X, CB_Y, "%s%s", "Your code is successfully added in codebase with key: ", key);
        refresh();
	getch();
	move(CB_X, CB_Y);
	clrtoeol();
        return;
}

void print_cbNotFound(char *key) {
	move(CB_X, CB_Y);
        clrtoeol();
        refresh();
        mvprintw(CB_X, CB_Y, "%s%s%s", "Oops! '", key, "' is not pressent in codebase...");
        refresh();
	getch();
        move(CB_X, CB_Y);
        clrtoeol();
        return;

}

void print_ReadOnly() {
        move(CB_X, CB_Y);
        clrtoeol();
        refresh();
        mvprintw(CB_X, CB_Y, "%s", "Read Only File, Cannot write on this file...");
        refresh();
        return;

}


int validate_codebase_key(char* key) {
	for(int i = 0; key[i] != '\0'; i++) {
		if((!isalnum(key[i])) && key[i] != '_')
			return 0;
	}
	return 1;
}

void init_gap_buff(line *lne) {
        // initially gap size = size of subline,
        // left will be at pos 0 and right at max-1
        lne->gap_left = 0;
        lne->gap_right = MAX_CHAR_IN_SUBLINE - 1;
        lne->gap_size = MAX_CHAR_IN_SUBLINE;
        return;
}


// Used to move the gap left in the array
// Assuming that position belong to given array
void move_gap_left(line *lne, int position) {
        char *buffer = lne->curr_line;
        int *gap_left = &(lne->gap_left);
        int *gap_right = &(lne->gap_right);

        if(lne->gap_size == 0) {
                *gap_left = position;   //TODO changed (don)
                *gap_right = position-1;
                return;
        }
        // Move the gap left character by character
        while (position < (*gap_left)) {
                (*gap_left)--;
                (*gap_right)--;
                buffer[(*gap_right) + 1] = buffer[*gap_left];
        }
        return;
}

// Used to move the gap right in the array
void move_gap_right(line *lne, int position) {
        // extract data from line structure to handle gap 
        char *buffer = lne->curr_line;
        int *gap_left = &(lne->gap_left);
        int *gap_right = &(lne->gap_right);

        // if gap size is zero
        if(lne->gap_size == 0) {
                if(position >= MAX_CHAR_IN_SUBLINE) {
                        *gap_left = MAX_CHAR_IN_SUBLINE;
                        *gap_right = MAX_CHAR_IN_SUBLINE - 1;
                        return;
                }
                *gap_left = position;
                *gap_right = position -1;
                return;
        }

        // Move the gap right character by character   
        while (position > (*gap_left)) {
                (*gap_left)++;
                (*gap_right)++;
                // For testing
                // TODO modify this
                if(*gap_right >= MAX_CHAR_IN_SUBLINE) {
                        (*gap_left)--;
                        (*gap_right)--;
                        //printf("already at last cannot move further right return from here\n");
                        return;
                }

                buffer[(*gap_left) - 1] = buffer[*gap_right];
        }
        return;
}


/* Move mouse cursor to particular position
 * RETURN:- line pointer with adjusting gap buffer, 
 * at that point data can be added or can be removed
 * NOTE:- position should belong to same block.
 */
line* move_cursor(line *subline, int position) {
	line *lne = subline;
	// take next subline till position doesn't belong to corresponding curr_line
	while((MAX_CHAR_IN_SUBLINE - lne->gap_size) < position) {
		// position not belong to current subline take next line
		// decrease position
		position -= (MAX_CHAR_IN_SUBLINE - lne->gap_size);
		// if current subline is last
		if(lne->rem_line == NULL)
			break;
		// take next subline
		lne = lne->rem_line;
	}
	
	// Now position belongs to current subline
	// move cursor(gap) at appropriate position
	if (position < lne->gap_left)
		move_gap_left(lne, position);
	else 
		move_gap_right(lne, position);
	return lne;
}
char** init_shortcut_keys() {
	char **shortcut_key = (char**)malloc(sizeof(char*) * TOT_SHORTCUT_KEYS);
        shortcut_key[0] = "printf(\"%\",);";
        shortcut_key[1] = "scanf(\"%\",);";
        shortcut_key[2] = "#include <>";
        shortcut_key[3] = "#define ";
        shortcut_key[4] = "int main() {\n    \n    return 0;\n}";
        shortcut_key[5] = "for(int i = ; )";
        shortcut_key[6] = "while()";
	shortcut_key[7] = "return ;";
	shortcut_key[8] = "typedef struct {\n    \n} ;";
	return shortcut_key;
}

int shortcut_key_indx(int *ch, int *move) {
        switch(*ch) {
               	case CTRL('p'):
                        *ch = SHORTCUT_KEY;
                        *move = 4;
                        return 0;
                case CTRL('a'):
                        *ch = SHORTCUT_KEY;
                        *move = 4;
                        return 1;
		case CTRL('e'):
                        *ch = SHORTCUT_KEY;
                        *move = 1;
                        return 2;
		case CTRL('d'):
                        *ch = SHORTCUT_KEY;
                        *move = 0;
                        return 3;
		case CTRL('n'):
                        *ch = SHORTCUT_KEY;
                        *move = 16;
                        return 4;
		case CTRL('f'):
                        *ch = SHORTCUT_KEY;
                        *move = 3;
                        return 5;
		case CTRL('w'):
                        *ch = SHORTCUT_KEY;
                        *move = 1;
                        return 6;
		case CTRL('r'):
                        *ch = SHORTCUT_KEY;
                        *move = 1;
                        return 7;
		case CTRL('t'):
                        *ch = SHORTCUT_KEY;
                        *move = 10;
                        return 8;
                default:
                        return -1;
        }
}


void check_bracket(int *ch, char *start, char *end) {
	int tmp = *ch;
	*ch = BRACKET;
	switch(tmp) {
		case '(':
			*start = '(';
			*end = ')';
			return;

		case '{':
			*start = '{';
			*end = '}';
			return;
			
		case '[':
			*start = '[';
			*end = ']';
			return;
		default:
			*ch = tmp;
			return;
	}
}

void insert_at_pos(line *subline, int position, char data) {
	// move the cursor at appropriate position
	line *lne = move_cursor(subline, position);

	// if current subline is full,
	// malloc new subline between current and next subline
	if(lne->gap_size == 0) {
		line *nl = (line*)malloc(sizeof(line));
		nl->curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
		nl->gap_left = 0;
		nl->gap_right = MAX_CHAR_IN_SUBLINE - 1;
		nl->gap_size = MAX_CHAR_IN_SUBLINE;

		int j = 0;
		for(int i = lne->gap_left; i < MAX_CHAR_IN_SUBLINE; i++) {
			// copy data from current subline to newly mallocated array
			// to grow gap
			insert_at_pos(nl, j, lne->curr_line[i]);
			lne->gap_size++;
			lne->gap_right++;
			j++;
		}
		// adjust pointers
		nl->rem_line = lne->rem_line;
		lne->rem_line = nl;

		// if cursor is at the last
		if(lne->gap_left >= MAX_CHAR_IN_SUBLINE)
			// take next subline
			lne = lne->rem_line;
	}

	// Insert the character in the gap and
	// move the gap
	lne->curr_line[lne->gap_left] = data;
	lne->gap_left++;
	lne->gap_size--;
	return;
}


// Use to delete one character from given position
// RETURN: deleted character
char del_from_pos(win *w, int *lne_no, int *col_no, FILE *fd_store_prev, FILE *fd_store_next, FILE *fd_main) {
	int line_no = *lne_no, position = *col_no;
	char data;
	// if position not 0th position
	if(position) {
		// decrease col_no(used in gui)
		(*col_no)--;

		int l_no = head_index(*w, line_no);

        	line *lne = &((w->head)[l_no].line);

		(w->head)[l_no].line_size--;		// decrease line size

		// Now position belongs to current subline
        	// move cursor(gap) at appropriate position
		lne = move_cursor(lne, position);
		// increase left boundary of gap buff to del char at given pos
		data = lne->curr_line[lne->gap_left - 1];
		lne->gap_left--;		//TODO decide what to do if only one character to remove now subline is empty
		lne->gap_size++;
		return data;
	}

	// delete character from position 0
	else {
		// if first line
		if(line_no == 0) {
			// check if line is present in filename_prev.tmp file
			if(! check_prev_line_available(fd_store_prev))
				return '\n';

			line tmp = w->head->line;
			int tmp_line_size = w->head->line_size;

			line *new_line = (line*)malloc(sizeof(line));
			// init new line
			new_line->curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
			// init next subline pointer with NULL
                        new_line->rem_line = NULL;
                        // init gap buffer
                        init_gap_buff(new_line);

			(w->head->line) = *new_line;
			*new_line = tmp;
			// init line size
			w->head->line_size = 0;

			char c;

			line *lne = &(w->head->line);
			// load previous line from file and update line_size
			w->head->line_size = prev_line_into_data_struct(lne, fd_store_prev);

			lne = &(w->head->line);
			while(lne->rem_line)
				lne = lne->rem_line;
			lne->rem_line = new_line;
			// adjust col_no(used in gui)
			*col_no = w->head->line_size;
			w->head->line_size += tmp_line_size;
			return '\n';
		}
		// not first line
		else {
			char ch;
	        	int extract_from_next = 0, not_available = 0;
		        int check = check_next_line_available(fd_store_next, fd_main);

			// next line not available
		        if(! check)
		                not_available = 1;
		        // available in next_tmp_file
		        if(check == 1)
                		extract_from_next = 1;

			// head_indx to current line(circular array)
			int l_no = head_index(*w, line_no);

			// malloc new line structure to join at end of previous line
			line *new_line = (line*)malloc(sizeof(line));
			*new_line = (w->head)[l_no].line;

			// add current line at the end of previous line
			int prev = head_index(*w, line_no - 1);
			line *lne = &( (w->head[prev]).line );
                        while(lne->rem_line)
                                lne = lne->rem_line;
                        lne->rem_line = new_line;

			// update co-ordinates(used in gui)
			(*lne_no)--;
			*col_no = (w->head[prev]).line_size;
			(w->head[prev]).line_size += (w->head[l_no]).line_size;

			int i;
			for(i = line_no; i < w->tot_lines - 1; i++) {
				int curr = head_index(*w, i), next = head_index(*w, i+1);
				// shift lines to make space at end to load next line from file
				w->head[curr].line = w->head[next].line;
				w->head[curr].line_size = w->head[next].line_size;
			}

			int last = head_index(*w, i);
			new_line = &(w->head[last].line);

			// init new line
                        new_line->curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
                        // init next subline pointer with NULL
                        new_line->rem_line = NULL;
                        // init gap buffer
                        init_gap_buff(new_line);

			// next line not available to load
			if(not_available) {
				new_line->curr_line[0] = MAX_CHAR;
				w->head[last].line_size = 0;
				return '\n';
			}

			// load next line from file and update line size
			w->head[last].line_size = next_line_into_data_struct(new_line, extract_from_next, fd_store_next, fd_main);
			return '\n';
		}

	}
}

void free_line(line* lne) {
        line* tmp_1 = lne->rem_line;
        line* tmp_2;
        while(tmp_1) {
                if(tmp_1->rem_line)
                        tmp_2 = tmp_1->rem_line;
                else tmp_2 = NULL;
                free(tmp_1);
                tmp_1->rem_line = NULL;
                tmp_1 = tmp_2;
        }
        lne->rem_line = NULL;
	free(lne->curr_line);
	lne->curr_line = NULL;
        return;
}


// extract line from ADT to tmp file
// suplimentary fun
void extract_line(node_l *tmp, FILE *fd_store) {
        line *data_line = &tmp->line;
	int indx = 0;

	// if line is empty means blank line, insert '\n' and return
	if(data_line->gap_size == MAX_CHAR_IN_SUBLINE && data_line->rem_line == NULL) {
		fputc('\n', fd_store);
		return;
	}

	if(data_line->gap_size != 0 && indx == data_line->gap_left)
		indx = data_line->gap_right + 1;

	char data;
	while(1) {
		//indx++;
		// skip gap
                if(data_line->gap_size != 0 && indx == data_line->gap_left)
                        indx = data_line->gap_right + 1;

		// if indx is at end of subline
		if(indx == MAX_CHAR_IN_SUBLINE) {
			// next subline not present append '\n' at end of line
			if(data_line->rem_line == NULL) {
				fputc('\n', fd_store);
				break;
			}
			// take next subline
			data_line = data_line->rem_line;
			indx = 0;
			if(data_line->gap_left == indx)		// if next next lines gap is at start(0th pos)
				continue;
		}

		if(data_line->gap_size == MAX_CHAR_IN_SUBLINE) { 	// TODO must be handle in del_from_pos fun
			indx = MAX_CHAR_IN_SUBLINE -1;
			continue;
		}
		data = data_line->curr_line[indx++];
                //printf("%c %d %d %d %d\n", data, data_line->gap_left, data_line->gap_right, data_line->gap_size, indx);
                // store data in tmp_prev file
                fputc(data, fd_store);
	}
	return;
}


// use to copy line from next line from file to give line in datastructure
// NOTE:- extract_from_next 1 if line to be extracted from tmp_next file
// else 0 if line to be extracted from main file
// line must be present in either of two files if not , condn should be handle by caller of this fun
// return number of characters loaded into line from file
int next_line_into_data_struct(line *lne, int extract_from_next, FILE *fd_store_next, FILE *fd_main) {
	char c;
	if(extract_from_next) {
		int count = 0;
                // if temp file in which next data is stored, has data extract from there,
                for(int i=0; ; i++) {
                        if(fseek(fd_store_next, -2, SEEK_CUR) == -1) {
                                fseek(fd_store_next, 0, SEEK_SET);      // goto start position
                                return count;
                        }
                        c = fgetc(fd_store_next);
                        if(c != '\n') {
                                // insert at position 0 because data is extracted in reverse passion
				insert_at_pos(lne, 0, c);
				count++;
                        }
                        else return count;
                }
        }

        // Now if line is not present in filename_nxt.tmp file
        // then extract from main file
        // next line not present
        else {
                int indx = 0;
                // insert till end of line
                while((c = fgetc(fd_main)) != '\n') {
			if(c == '\t')
				for(int i = 0; i < TAB_SPACE; i++)
					insert_at_pos(lne, indx++, ' ');
			else insert_at_pos(lne, indx++, c);
                }
                return indx;
        }
}


// use to copy line from prev line from file to give line in datastructure
// line must be present in either of two files if not , condn should be handle by caller of this fun
// return number of characters loaded into line from file
int prev_line_into_data_struct(line *lne, FILE *fd_store_prev) {
	int count = 0;
	char c;
        // if temp file in which previous data is stored, has data extract from there,
        for(int i=0; ; i++) {
                if(fseek(fd_store_prev, -2, SEEK_CUR) == -1) {
                        fseek(fd_store_prev, 0, SEEK_SET);      // goto start position
                        return count;
                }

                c = fgetc(fd_store_prev);
                if(c != '\n') {
                        insert_at_pos(lne, 0, c);
                        count++;
                }
                else return count;
        }
}


/* func to check next line is available or not
 * return 0 if not available
 * return 1 if available in next tmp_file
 * reutrn 2 if not available in next tmp_file but available in main file
 * */
int check_next_line_available(FILE *fd_store_next, FILE *fd_main) {
	char ch;
        if(fseek(fd_store_next, -1, SEEK_CUR) != -1) {
                fseek(fd_store_next, 1, SEEK_CUR);
		return 1;
        }
        else if((ch = fgetc(fd_main)) == -1) {
                // next line not available
                return 0;
        }
        ungetc(ch, fd_main);    // restoring previous condition
	return 2;
}


/* func to check next line is available or not
 * return 0 if not available
 * return 1 if available
 */
int check_prev_line_available(FILE *fd_store_prev) {
	// check if line is present in filename_prev.tmp file
        if(fseek(fd_store_prev, -1, SEEK_CUR) != -1) {    // present
                fseek(fd_store_prev, 1, SEEK_CUR);
		return 1;
	}
        // else return as line not present to load previous line
        return 0;
}


/*
 * used to load next line at last, after removing first line.
 * fd_store_prev : file descripter to store line which is just to be removed from top of ADT
 * fd_store_next : file descripter to store line which is just to be removed from bottom of ADT
 * fd_get   : file descripter to pointing in main file.
 * RETURN: 0 for success, 1 for failure(if next line not available)
 */
int load_next_line(win *w, FILE *fd_store_prev, FILE *fd_store_next, FILE *fd_main) {
	char ch;
	int extract_from_next = 0;
	int check = check_next_line_available(fd_store_next, fd_main);
	// next line not available
	if(! check)
		return FAILURE;
	// available in next_tmp_file
	if(check == 1)
		extract_from_next = 1;

	/*extract first line of window from ADT to tmp file*/
        if(w->head->line.curr_line[0] != MAX_CHAR)
                extract_line(w->head, fd_store_prev);
	else return FAILURE;	//TODO maybe this case will not arise

	// free mallocated sublines corresponding to data_line in ADT
	free_line(&w->head->line);	//TODO: try to do without free

	// init new line at head
	w->head->line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
	w->head->line.rem_line = NULL;
	// init gap buffer of new line
	init_gap_buff(&w->head->line);
	// init line size
	w->head->line_size = 0;

	// load next line from file
	line *lne = &(w->head)->line;
	char c;

	// load next line and change line size accordingly
	w->head->line_size = next_line_into_data_struct(lne, extract_from_next, fd_store_next, fd_main);

	w->head_indx = (w->head_indx + 1) % w->tot_lines;

	if(w->head_indx == 0)
		w->head -= (w->tot_lines-1);
	else w->head++;

	return SUCCESS;
}


int load_prev_line(win *w, FILE *fd_store_prev, FILE *fd_store_next) {
	// check previous line is available or not
	if(! check_prev_line_available(fd_store_prev))
		return FAILURE;

	// point head pointer to previous line
	if(w->head_indx == 0) {
		w->head_indx = w->tot_lines - 1;
		w->head += (w->tot_lines - 1);
	}
	else {
		w->head_indx--;
		w->head--;
	}

	// extract last line into filename_next.tmp
        if(w->head->line.curr_line[0] != MAX_CHAR)
                extract_line(w->head, fd_store_next);
        // free mallocated sublines corresponding to data_line in ADT
        free_line(&w->head->line);	// TODO: try to do without free

	// init new line at head
        w->head->line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
        w->head->line.rem_line = NULL;
        // init gap buffer of new line
        init_gap_buff(&w->head->line);
	// init line size
	w->head->line_size = 0;

        // load prev line from file
        line *lne = &(w->head)->line;
	w->head->line_size = prev_line_into_data_struct(lne, fd_store_prev);
	return SUCCESS;
}


// use to return head_index
int head_index(win w, int line_no) {	//TODO: use in print fun
	 int h_indx = line_no;
	 // circular array
	 if(h_indx + w.head_indx >= w.tot_lines)
		 h_indx = h_indx - w.tot_lines;
	 return h_indx;
}

// NOTE:- first line is 0th line
void insert_new_line_at_pos(win *w, int *lne_no, int *col_no, FILE *fd_prev, FILE *fd_nxt, FILE *fd_main) {
	int line_no = *lne_no, position = *col_no;
	// not last line
	if(line_no != w->tot_lines - 1) {
		int last = head_index(*w, w->tot_lines-1);

		extract_line(&(w->head[last]), fd_nxt);
		free_line(&(w->head[last]).line);

		for(int i = w->tot_lines-1; i > line_no; i--) {
			int curr = head_index(*w, i-1);
	                int next = head_index(*w, i);

			(w->head[next]).line = (w->head[curr]).line;
			(w->head[next]).line_size = (w->head[curr]).line_size;
		}

		int empty = head_index(*w, line_no + 1);
		// init new line
		(w->head)[empty].line_size = 0;
                (w->head)[empty].line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
                // init next subline pointer with NULL
                (w->head)[empty].line.rem_line = NULL;
                // init gap buffer
                init_gap_buff(&(w->head[empty]).line);

		int current = head_index(*w, line_no);
		line *lne = move_cursor(&(w->head)[current].line, position);
		position = lne->gap_right + 1;

		// insert data in next blank line
                int indx = 0;
                while(position < MAX_CHAR_IN_SUBLINE) {
                        if(position == lne->gap_left)
                                position = lne->gap_right + 1;

                        insert_at_pos(&((w->head)[empty].line), indx++, lne->curr_line[position++]);
                }

		// update line size of both lines
		(w->head[empty]).line_size = (w->head[current]).line_size - *col_no;
		(w->head[current]).line_size = *col_no;

		// if next subline is present then point rem_line pointer to that subline
                // so that copy operation will be saved
                (w->head)[empty].line.rem_line = lne->rem_line;
		lne->gap_size += indx;   // change gap_size
                // gap_right = end to indicate all data after position is deleted
                lne->gap_right = MAX_CHAR_IN_SUBLINE - 1;
                lne->rem_line = NULL;

		// update co ordinates(used in gui)
                (*lne_no)++;
                *col_no = 0;
	}
	// last line
	else {
		extract_line(w->head, fd_prev);
		free_line( &(w->head->line) );

                // shift line pointer by one unit to create space for blank line
                for(int i = 0; i < (w->tot_lines-1); i++) {
			int curr = head_index(*w, i);
                        int next = head_index(*w, i+1);

			(w->head)[curr].line = (w->head)[next].line;
			(w->head)[curr].line_size = (w->head)[next].line_size;
                }

		int current = head_index(*w, line_no);
		int prev = head_index(*w, line_no-1);
		// init new line
                (w->head)[current].line.curr_line = (char*)malloc(sizeof(char) * MAX_CHAR_IN_SUBLINE);
                // init next subline pointer with NULL
                (w->head)[current].line.rem_line = NULL;
                // init gap buffer
                init_gap_buff(&(w->head[current]).line);

		// line in which new line to be inserted
                line *lne = &(w->head[prev]).line;

		// move cursor to appropriate position with adjusted gap buffer at position
		lne = move_cursor(lne, position);
                position = lne->gap_right + 1;

                // insert data in next blank line
                int indx = 0;
                while(position < MAX_CHAR_IN_SUBLINE) {
                        if(position == lne->gap_left)
                                position = lne->gap_right + 1;

                        insert_at_pos( &((w->head)[current].line), indx++, lne->curr_line[position++] );
                }

		// update line size of both lines
		(w->head[prev]).line_size = *col_no;
                (w->head[current]).line_size = (w->head[current]).line_size - *col_no;

                (w->head)[current].line.rem_line = lne->rem_line;

                // if next subline is present then point rem_line pointer to that subline
                // so that copy operation will be saved
                lne->gap_size += indx;   // change gap_size
                // gap_right = end to indicate all data after position is deleted
                lne->gap_right = MAX_CHAR_IN_SUBLINE - 1;
                lne->rem_line = NULL;

		// update co ordinates(used in gui)
                *col_no = 0;
	}
	return;
}


void print_loc(int x, int y) {
	move(LOC_X, LOC_Y);
	clrtoeol();
        mvprintw(LOC_X, LOC_Y, "x: %d y: %d", x, y);
}

void print_debug(int x, int y) {
        mvprintw(1, 90, "k: %d b: %d", x, y);
}


/*use to print contents of ADT - for testing*/
void print_page(win w, TrieNode *keyword) {
	line *lne;
	int h_indx;
	char word_arr[100];
	for(int i = 0; i < w.tot_lines; i++) {
		// for keyword coloring
		int windx = 0;
		char comment = 0;
		char color = WHITE;

		h_indx = head_index(w, i);

		lne = &((w.head)[h_indx].line);
		char c = 1;
		int indx = 0;

		if(lne->curr_line[0] == MAX_CHAR) {
			for(; i < w.tot_lines; i++) {
				h_indx = head_index(w, i);
				lne = &((w.head)[h_indx].line);
				mvaddch(i, 0, TILDE);
			}
			refresh();
                        return;
		}

		// to clear previously written line from screen
		move(i, 0);
                clrtoeol();
		int col = 0;
	
		// 3(orange), 15(white), 9(red), 10(light_green), 11(yerllow), 21(dark blue), 39(light_blue)

		int brk = 0;
                while(1) {
                        if(lne->gap_size != 0 && indx == lne->gap_left)
				indx = lne->gap_right + 1;
			if(indx == MAX_CHAR_IN_SUBLINE) {
				if(lne->rem_line == NULL) {
					brk = 1;
					goto LABEL;
					// new line
				}
                                lne = lne->rem_line;
                                indx = 0;
				continue;
                        }
			
			c = lne->curr_line[indx++];
			if(c == '/' && comment == 0)
				comment = '/';
			else if(c == '/' && comment == '/') {
				comment = 1;
				attron(COLOR_PAIR(COMMENT));
                                mvaddch(i, col++, '/');
                                attroff(COLOR_PAIR(COMMENT));
			}			
	
		LABEL:
			if(comment == 1) {
				if(brk)
					break;
				attron(COLOR_PAIR(COMMENT));
                                mvaddch(i, col++, c);
                                attroff(COLOR_PAIR(COMMENT));
			}
			else if(c == '#' || c == '<' || c == ' ' || c == '(' || c == ';' || brk) {
				word_arr[windx++] = '\0';
				if(! search(keyword, word_arr, &color))
					color = WHITE;

				attron(COLOR_PAIR(color));
				for(int k = 0; word_arr[k] != '\0'; k++)
					mvaddch(i, col++, word_arr[k]);
				attroff(COLOR_PAIR(color));
				
				if(brk)
					break;
				mvaddch(i, col++, c);
				windx = 0;
			}
			else
				word_arr[windx++] = c;
                }
        }
	refresh();
	return;
}


void print_line(win w, int line_no, TrieNode *keyword) {
	line *lne;
        int h_indx;
        char word_arr[100];

	// for keyword coloring
	int windx = 0;
	char comment = 0;
	char color = WHITE;

	h_indx = head_index(w, line_no);

	lne = &((w.head)[h_indx].line);
	char c = 1;
	int indx = 0;

	if(lne->curr_line[0] == MAX_CHAR) {
		mvaddch(line_no, 0, TILDE);
		refresh();
		return;
	}

	// to clear previously written line from screen
	move(line_no, 0);
	clrtoeol();
	int col = 0;

	int brk = 0;
	while(1) {
		if(lne->gap_size != 0 && indx == lne->gap_left)
			indx = lne->gap_right + 1;
		if(indx == MAX_CHAR_IN_SUBLINE) {
			if(lne->rem_line == NULL) {
				brk = 1;
				goto LABEL;
			}
			lne = lne->rem_line;
			indx = 0;
			continue;
		}

		c = lne->curr_line[indx++];
		if(c == '/' && comment == 0)
			comment = '/';
		else if(c == '/' && comment == '/') {
			comment = 1;
			attron(COLOR_PAIR(COMMENT));
			mvaddch(line_no, col++, '/');
			attroff(COLOR_PAIR(COMMENT));
		}

	    LABEL:
		if(comment == 1) {
			if(brk)
				break;
			attron(COLOR_PAIR(COMMENT));
			mvaddch(line_no, col++, c);
			attroff(COLOR_PAIR(COMMENT));
		}        		
		else if(c == '#' || c == '<' || c == ' ' || c == '(' || c == ';' || brk) {
			word_arr[windx++] = '\0';
			if(! search(keyword, word_arr, &color))
				color = WHITE;

			attron(COLOR_PAIR(color));
			for(int k = 0; word_arr[k] != '\0'; k++)
				mvaddch(line_no, col++, word_arr[k]);
			attroff(COLOR_PAIR(color));

			if(brk)
				break;
			mvaddch(line_no, col++, c);
			windx = 0;
		}
		else
			word_arr[windx++] = c;
	}
	refresh();
	return;
}


// for print output on terminal, for debugging
void print(win w) {
        line *lne;
        int h_indx;
	int a = 0;
        for(int i = 0; i < w.tot_lines; i++) {
                h_indx = i;
                // circular array
                if(h_indx + w.head_indx >= w.tot_lines)
                        h_indx = h_indx - w.tot_lines;

                lne = &((w.head)[h_indx].line);
                char c = 1;
                int indx = 0;

                if(lne->curr_line[0] == MAX_CHAR)
                        return;

		while(1) {
                        if(lne->gap_size != 0 && indx == lne->gap_left)
                                indx = lne->gap_right + 1;
			if(indx == MAX_CHAR_IN_SUBLINE) {
                                if(lne->rem_line == NULL) {
                                        printf("\n");
                                        break;
                                }
                                lne = lne->rem_line;
                                indx = 0;
				continue;
                        }
                        c = lne->curr_line[indx++];
			printf("%c", c);
		}
        }
        return;
}


int main(int argc, char *argv[]) {
	// if file name not entered
	if(argc < 2) {
		printf("Please Enter filename.\n");
		return 0;
	}

	FILE *fd_store_prev, *fd_store_next, *fd_main;
	win window_1;
	// initialize window
        init_window(&window_1, TOT_LINES_IN_WINDOW);
	// init stack for undo operation
        stack st;
        init(&st);
	
	// init codebase files
        FILE *fd_cb_key, *fd_cb_data;
        fd_cb_key = fopen(".codebase.key", "r+");
        fd_cb_data = fopen(".codebase.data","r+");
        int codebase_mode = 0;
	// init codebase into trie
        TrieNode_c *codebase = init_codebase(fd_cb_key);


	int new_file = 0;
	// check file corresponding to input 
	// filename is present or not
	struct stat perm;
	if(stat(argv[1], &perm) == -1)
		new_file = 1;

	// if file is directory, cannot open
	if(S_ISDIR(perm.st_mode)) {
		printf("Cannot Open Directory.\n");
		return 0;
	}

	// check whether file has write permission or not
	int read_only = 0;
	if(! (perm.st_mode & S_IWUSR))
                read_only = 1;
	
	// if file not present create new with 'read write' permission
	if(new_file)
		fd_main = fopen(argv[1], "w+");
	// else load file into data structure
	else
		fd_main = load_file(&window_1, argv[1]);

	char prev_file[50] = ".";
	char next_file[50] = ".";
	// generate tmp filenames, filename_prev.tmp and filename_next.tmp
	strcat(strcat(prev_file, argv[1]), "_prev.tmp");
	strcat(strcat(next_file, argv[1]), "_next.tmp");

	// open tmp files
        fd_store_prev = fopen(prev_file, "w+");
        fd_store_next = fopen(next_file, "w+");	

	
	// curses interface 
        initscr();
        noecho();
        keypad(stdscr, true);

	// init colors for syntax hilighting
	init_colors();
	TrieNode *keyword = init_keywords();

	// init shortcut keys
	char **shortcut_key = init_shortcut_keys();

	int ch;
	int win_line = 0, win_col = 0, line_no = 0;
	int pos_changed = 0;

	// print whole page at start
	print_page(window_1, keyword);
	if(read_only)
		print_ReadOnly();
	// print co ordinates
	print_loc(line_no, win_col);
	// move cursor
	move(line_no, win_col);
	refresh();

	int cnt = 0;
	while(true) {
		// take user input
		ch = getch();
		cnt++;

		int move_left = 0;
		char start_bracket, end_bracket;
		// check if input is any shortcut key
		int sk_index = shortcut_key_indx(&ch, &move_left);
		// check if input key is bracket
		check_bracket(&ch, &start_bracket, &end_bracket);

		int up = 1, down = 1;
		if(ch == KEY_NPAGE) { 
			ch = KEY_DOWN;
			down = window_1.tot_lines-1;

			line_no += window_1.tot_lines-1 - win_line;
                        win_line = window_1.tot_lines-1;

			int h_indx = head_index(window_1, win_line);
			if(win_col > (window_1.head)[h_indx].line_size)
				win_col = (window_1.head)[h_indx].line_size;
		}
		else if(ch == KEY_PPAGE) {
			ch = KEY_UP;
			up = window_1.tot_lines-1;

			line_no -= win_line;
			win_line = 0;

			int h_indx = head_index(window_1, win_line);
			if(win_col > (window_1.head)[h_indx].line_size)
				win_col = (window_1.head)[h_indx].line_size;
		}
		
		switch(ch) {
			// quit without save
			case 'q':
				// close all files
	                	fclose(fd_main);
	        	        fclose(fd_store_prev);
        	        	fclose(fd_store_next);
				fclose(fd_cb_key);
		                fclose(fd_cb_data);

				// remove all extra files
				if(new_file)
        		        	remove(argv[1]);
	                	remove(prev_file);
                		remove(next_file);

				// end ncurses interface
				endwin();
				return 0;

			// save and quit
			case CTRL('y'):
				goto SAVE;

			// undo function
			case KEY_F(4):
				undo(&st, &window_1, &line_no, &win_line, &win_col, fd_store_prev, fd_store_next, fd_main);
				print_page(window_1, keyword);
				break;

			// left arrow key
			case KEY_LEFT:
				// decrese column no if not 0
				pos_changed = 1;
	                        if(win_col)
        	                        win_col--;
                  	      	break;

			// right arrow key
			case KEY_RIGHT:
				// increase column no if cursor not at end of line
				pos_changed = 1;
	                        if(win_col < (window_1.head)[head_index(window_1, win_line)].line_size)
        	                        win_col++;
                	        break;

			
			// down arrow key
			case KEY_DOWN:
				for(int k = 0; k < down; k++) {
				pos_changed = 1;
                        	if(win_line < window_1.tot_lines - 1) {
					// increase line no
                                	win_line++;
                                	line_no++;

                                	int h_indx = head_index(window_1, win_line);
					// update column no
                               		if(win_col > (window_1.head)[h_indx].line_size)
                       	        	        win_col = (window_1.head)[h_indx].line_size;

					if((window_1.head)[h_indx].line.curr_line[0] == MAX_CHAR) {
						win_line--;
						line_no--;
					}

                	        }
				// if down arrow at bottom of window, load next line
        	                else {
					// load next line
	                                int check = load_next_line(&window_1, fd_store_prev, fd_store_next, fd_main);
					// if next line successfully loaded
                                	if(check == SUCCESS) {
                        	                line_no++;

                	                	int h_indx = head_index(window_1, win_line);
						// adjust column number
						// adjust column no
        	                        	if(win_col > (window_1.head)[h_indx].line_size)
	                                        	win_col = (window_1.head)[h_indx].line_size;

						// store info in stack for undo function
                                		store_info(&st, 0, ch, LOAD_NEXT_LINE, win_line, win_col);
					}
					// print updated page on screen
					print_page(window_1, keyword);
                        	}
				}
                        	break;

			// up arrow key
			case KEY_UP:
				for(int k = 0; k < up; k++) {
				pos_changed = 1;
				// if curr line not first
                        	if(win_line > 0) {
					// decrease line no
                	                win_line--;
        	                        line_no--;
					
					// adjust column no
        	                        int h_indx = head_index(window_1, win_line);
	                                if(win_col > (window_1.head)[h_indx].line_size)
                                	        win_col = (window_1.head)[h_indx].line_size;

                        	}
				// if up arrow pressed at top of window, load prev line
                        	else {
                                	int check = load_prev_line(&window_1, fd_store_prev, fd_store_next);
					// if prev line successfully loaded
                                	if(check == SUCCESS) {
						// update line no and column no
                                        	line_no--;

                                		int h_indx = head_index(window_1, win_line);
                                		if(win_col > (window_1.head)[h_indx].line_size)
                                	 	       win_col = (window_1.head)[h_indx].line_size;

						// store info in stack
						store_info(&st, 0, ch, LOAD_PREV_LINE, win_line, win_col);
					}
					// print updated page on screen
					print_page(window_1, keyword);
                        	}
				}
                        	break;

			case KEY_BACKSPACE: {
				if(read_only)
					goto READ_ONLY;
        	                // at start of file, do nothing
	                        if(line_no == 0 && win_col == 0)
                        	        continue;

                	        char operation = DEL_CHAR;
        	                if(win_col == 0) {
	                                operation = DEL_LINE;
                                	line_no--;
                        	}
                        	char data = del_from_pos(&window_1, &win_line, &win_col, fd_store_prev, fd_store_next, fd_main);
                        	store_info(&st, pos_changed, data, operation, win_line, win_col);
                	        pos_changed = 0;

				if(operation == DEL_LINE)
					print_page(window_1, keyword);
				else
					print_line(window_1, win_line, keyword);
	                        break;
        	        }

			// enter key
			case '\n':
				if(read_only)
					goto READ_ONLY;
	                        line_no++;
                        	insert_new_line_at_pos(&window_1, &win_line, &win_col, fd_store_prev, fd_store_next, fd_main);
        	                store_info(&st, pos_changed, ch, INSERT_NEW_LINE, win_line, win_col);
                	        pos_changed = 0;
				// print updated page on screen
				print_page(window_1, keyword);
	                        break;
			
			case '\t': {
				if(read_only)
                                        goto READ_ONLY;
				int h_indx = head_index(window_1, win_line);
                                for(int i = 0; i < TAB_SPACE; i++) {
                                        (window_1.head)[h_indx].line_size++;
                                        insert_at_pos(&((window_1.head)[h_indx].line), win_col++, ' ');
                                        store_info(&st, pos_changed, ch, INSERT_CHAR, win_line, win_col);
                                        if(pos_changed)
                                                pos_changed = 0;
                                }
				print_line(window_1, win_line, keyword);
				break;
			}

			// insert bracket with pair
			case BRACKET: {
				if(read_only)
                                        goto READ_ONLY;
				// insert bracket with pair
				int h_indx = head_index(window_1, win_line);
                                (window_1.head)[h_indx].line_size += 2;
                                insert_at_pos(&((window_1.head)[h_indx].line), win_col++, start_bracket);
				insert_at_pos(&((window_1.head)[h_indx].line), win_col++, end_bracket);
                                store_info(&st, pos_changed, ch, INSERT_CHAR, win_line, win_col);
				win_col--;
                                pos_changed = 0;

				print_line(window_1, win_line, keyword);
				break;
			}
		
			// insert ; at end
			case CTRL(';'): {
				int h_indx = head_index(window_1, win_line);
				insert_at_pos(&((window_1.head)[h_indx].line), (window_1.head)[h_indx].line_size++, ';');
				
				print_line(window_1, win_line, keyword);
				break;
			}

			// move to start of line
			case CTRL('h'):
				pos_changed = 1;
				win_col = 0;
				break;
			// move to end of line
			case CTRL('l'):
				pos_changed = 1;
                                win_col = (window_1.head)[head_index(window_1, win_line)].line_size;
                                break;
			// move to top line of window
			case CTRL('o'): {
				pos_changed = 1;
				line_no -= win_line;
                                win_line = 0;

                                int h_indx = head_index(window_1, win_line);
                                if(win_col > (window_1.head)[h_indx].line_size)
                                        win_col = (window_1.head)[h_indx].line_size;
                                break;
			}
			// move to bottom line of window
                        case CTRL('k'): {
				pos_changed = 1;
				line_no += window_1.tot_lines-1 - win_line;
				win_line = window_1.tot_lines-1;

				int h_indx = head_index(window_1, win_line);
				if(win_col > (window_1.head)[h_indx].line_size)
					win_col = (window_1.head)[h_indx].line_size;
                                break;
			}	

			// shortcut keys to insert common 'c' syntax
			case SHORTCUT_KEY: {
				if(read_only)
                                        goto READ_ONLY;
				// shortcut_key[sk_index] will give text 
				// which is to be inserted at current position
				for(int i = 0; shortcut_key[sk_index][i] != '\0'; i++) {
					int h_indx = head_index(window_1, win_line);

					if(shortcut_key[sk_index][i] == '\n') {
						line_no++;
		                                insert_new_line_at_pos(&window_1, &win_line, 
								&win_col, fd_store_prev, fd_store_next, fd_main);
                		                store_info(&st, pos_changed, ch, INSERT_NEW_LINE, win_line, win_col);
					}
					else {
						(window_1.head)[h_indx].line_size++;
	                                	insert_at_pos(&((window_1.head)[h_indx].line), win_col++, shortcut_key[sk_index][i]);
        	                        	store_info(&st, pos_changed, ch, INSERT_CHAR, win_line, win_col);
					}
					if(pos_changed)
						pos_changed = 0;
				}
				// move cursor at appropriate position as requirment of syntax
				for(int i = 0; i < move_left; i++) {
                                        win_col--;
					if(win_col < 0) {
						line_no--;
						win_line--;
						win_col = (window_1.head[head_index(window_1, win_line)]).line_size;
					}
				}

				print_page(window_1, keyword);
				break;
			}

		CODEBASE_KEY_MODE:
			case KEY_F(1): {
				// accept input key from user
				char *key = accept_codebase_key(win_line, win_col);
				// validate key
				if(! validate_codebase_key(key)) {
					// if invalid print msg on screen
					print_cbError();
					ch = getch();
					// if retry option selected, goto start of this block
					if(ch == KEY_F(1)) {
						move(CB_X, CB_Y);
						clrtoeol();
						goto CODEBASE_KEY_MODE;
					}
					// user dont want to retry, back to editor
					else {
						move(CB_X, CB_Y);
                                                clrtoeol();
						move(win_line, win_col);
						break;
					}
				}
				// key is valid, accept data to store in codebase
				print_cbAccept();
				
				// goto end to store data
				fseek(fd_cb_data, 0, SEEK_END);
				// store lower(start) offset of data
                                long int lower = ftell(fd_cb_data);
				// fill data
				while((ch = getch()) != KEY_F(2)) {
					fputc(ch, fd_cb_data);
				}
				// store upper(end) offset of data
				long int upper = ftell(fd_cb_data);
				// insert key in trie
				insert_in_codebase(codebase, key, (int)lower, (int)upper);
				// insert key in codebase file
				store_key(fd_cb_key, key, (int)lower, (int)upper);

				print_cbSuccess(key);

				break;
			}

			case KEY_F(3): {
				if(read_only)
                                        goto READ_ONLY;
				// accept key to search in trie
				char *key = accept_codebase_key(win_line, win_col);
				int lower, upper;
				// search input key
			  	if(search_in_codebase(codebase, key, &lower, &upper)) {
					// goto start of data
					fseek(fd_cb_data, lower, SEEK_SET);
					while(ftell(fd_cb_data) != upper) {
						ch = fgetc(fd_cb_data);

						// insert text in data structure
						if(ch != '\n') {
							int h_indx = head_index(window_1, win_line);
	                	        	        (window_1.head)[h_indx].line_size++;
        	                	        	insert_at_pos(&((window_1.head)[h_indx].line), win_col++, ch);
	                	                	store_info(&st, pos_changed, ch, INSERT_CHAR, win_line, win_col);
        	         	       		        pos_changed = 0;
						}
						else {
							line_no++;
			                                insert_new_line_at_pos(&window_1, &win_line, &win_col, 
									fd_store_prev, fd_store_next, fd_main);
                        			        store_info(&st, pos_changed, ch, INSERT_NEW_LINE, win_line, win_col);
							pos_changed = 0;
						}
					}
				}
				else print_cbNotFound(key);
			}
				print_page(window_1, keyword);
				break;

			// insert input text in data structure
			default:{
				if(read_only)
                                        goto READ_ONLY;
                        	int h_indx = head_index(window_1, win_line);
                        	(window_1.head)[h_indx].line_size++;
                        	insert_at_pos(&((window_1.head)[h_indx].line), win_col++, ch);
                        	store_info(&st, pos_changed, ch, INSERT_CHAR, win_line, win_col);
                        	pos_changed = 0;

				print_line(window_1, win_line, keyword);
			}
		}

	READ_ONLY:
		print_loc(line_no, win_col);
		move(win_line, win_col);
		refresh();
	}

	SAVE:
		endwin();

		// retrive contents from fd_store_prev file
		long tmp;
		// new tempory file to generate file from tmp files and 
		// buffer at time of save
		FILE *fd_new = fopen("suraj.c", "w+");
		// save offset prev file
		tmp = ftell(fd_store_prev);

		fseek(fd_store_prev, SEEK_SET, 0);

		// from start to tmp offset
		for(int i = 0; i != tmp; i++)
			fputc(fgetc(fd_store_prev), fd_new);


		// retrive contents from window buffer
		line *lne;
		for(int i = 0; i < window_1.tot_lines; i++) {
			int h_indx = head_index(window_1, i);
			lne = &((window_1.head)[h_indx].line);
                	char c = 1;
	                int indx = 0;

			// next line not available
        	        if(lne->curr_line[0] == MAX_CHAR)
                	        break;

			while(1) {
                        	if(lne->gap_size != 0 && indx == lne->gap_left)
                                	indx = lne->gap_right + 1;
	                        if(indx == MAX_CHAR_IN_SUBLINE) {
        	                        if(lne->rem_line == NULL) {
                        	                fputc('\n', fd_new);
                	                        break;
                                	}
	                                lne = lne->rem_line;
        	                        indx = 0;
                	                continue;
                        	}
	                        c = lne->curr_line[indx++];
        	                fputc(c, fd_new);
                	}
		}	

	
		long tmp_offset;
		char c;
		while(1) {
                       	if(fseek(fd_store_next, -2, SEEK_CUR) == -1) {
				if(fseek(fd_store_next, -1, SEEK_CUR) == -1)
					break;
				while((c = fgetc(fd_store_next)) != '\n')
                                        fputc(c, fd_new);
                                fputc('\n', fd_new);
	                        break;
			}
			c = fgetc(fd_store_next);
			if(c == '\n') {
				tmp_offset = ftell(fd_store_next) - 1;
				while((c = fgetc(fd_store_next)) != '\n')
					fputc(c, fd_new);
				fputc('\n', fd_new);

				fseek(fd_store_next, tmp_offset, SEEK_SET);
				//printf("%ld %c\n", tmp_offset, fgetc(fd_store_next));
			}
		}


                while((c = fgetc(fd_main)) != -1)
                        fputc(c, fd_new);


		stat(argv[1], &perm);
		chmod("suraj.c", perm.st_mode);

		fclose(fd_new);
		fclose(fd_main);
                fclose(fd_store_prev);
                fclose(fd_store_next);
		fclose(fd_cb_key);
		fclose(fd_cb_data);

		remove(argv[1]);
		remove(prev_file);
		remove(next_file);

		rename("suraj.c", argv[1]);

	return 0;
}