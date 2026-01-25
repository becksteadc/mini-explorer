#define UI_IMPLEMENTATION
#define UI_WINDOWS
#include "luigi.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

UITextbox *textbox;
UIButton *button;
UILabel *label;
UITable *dir_listing;

struct String_Linked_List {
	char *str;
	size_t length;
	struct String_Linked_List *next;
};

struct String_Linked_List Global_Dir_Listing;
/* Event callback functions. (listeners) */
int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp);
int TextboxMessage(UIElement *element, UIMessage message, int di, void *dp);
int TableMessage(UIElement *element, UIMessage message, int di, void *dp);
int TableContents(UIElement *element, UIMessage message, int di, void *dp);

/* background functionality & helper functions. */
int list_dir(const char *path, struct String_Linked_List *result);
int pack_SLL_to_CString(struct String_Linked_List *head, char **result);
void cleanup_SLL(struct String_Linked_List *list);

void cwb_init_globals(void)
{
	Global_Dir_Listing.str = "Default";
	Global_Dir_Listing.length = sizeof("Default");
	Global_Dir_Listing.next = NULL;
}

/* Entry Point -- for linux support, add conditional compilation here. */
int WinMain(HINSTANCE instance, HINSTANCE previousInstance,
			LPSTR commandline, int showCommand)
{
	UIInitialise();
	cwb_init_globals();
	UIWindow *window = UIWindowCreate(0, 0, "hello", 640, 480);
	UISplitPane *split1 = UISplitPaneCreate(&window->e, UI_SPLIT_PANE_VERTICAL, 0.5f);
	dir_listing = UITableCreate(&split1->e, 0, "File Name\tFile Permissions"); 
	UIPanel *panel = UIPanelCreate(&split1->e, UI_PANEL_GRAY |
									UI_PANEL_MEDIUM_SPACING);
	textbox = UITextboxCreate(&panel->e, 0);
	button = UIButtonCreate(&panel->e, 0, "Push", -1);
	label = UILabelCreate(&panel->e, 0, "text", -1);
	

	button->e.messageUser = ButtonMessage;
	textbox->e.messageUser = TextboxMessage;
	//dir_listing->e.messageUser = TableMessage;
	dir_listing->e.messageUser = TableContents;
	dir_listing->itemCount = 2; //debug only for now -- TODO
	UITableResizeColumns(dir_listing);
	

	return UIMessageLoop();
}

void cleanup_SLL(struct String_Linked_List *l)
{
	if (l == NULL) return;

	struct String_Linked_List *curr = l;
	struct String_Linked_List *next_node;
	while (curr->next != NULL) {
		next_node = curr->next;
		free(curr->str);
		free(curr);
		curr = next_node;
	}
	free(curr->str);
	free(curr);
	return;
}

//replaces the duplicated functionality in list_dir
//TODO - actually finish refactoring that: remove the duplicated code
//TODO - transition to using linked list + table in gui rather than string + code block
int create_dir_SLL(const char *path, struct String_Linked_List *result)
{
	struct dirent *entry;
	DIR *dp;
	dp = opendir(path);
	if (dp == NULL) {
		perror("create_dir_sll: Unable to open directory.");
		return -1;
	}
	#define MAX_FNAME_LENGTH 256 
	char buf[MAX_FNAME_LENGTH];
	struct String_Linked_List *list_head = NULL;
	struct String_Linked_List *list_current_pos = NULL;
	while ((entry = readdir(dp))) {
		if (list_head == NULL) {
			list_head = (struct String_Linked_List *) malloc(sizeof(struct String_Linked_List));
			list_head->length = strlen(entry->d_name);
			list_head->str = (char *) malloc(sizeof(char) * list_head->length + 1);
			strcpy(list_head->str, entry->d_name);
			list_head->next = NULL;
			list_current_pos = list_head;
		} else {
			struct String_Linked_List *new = malloc(sizeof(struct String_Linked_List)); //MALLOC --
			list_current_pos->next = new;
			new->length = strlen(entry->d_name);
			new->str = (char *) malloc(sizeof(char) * new->length + 1); //MALLOC --
			strcpy(new->str, entry->d_name);
			new->next = NULL;

			list_current_pos = new; //defer'd / must be at end of else block
		}
	}

	result = list_head; //"return" the result
	return 0;
} 

//caller must free returned result
//returns 0 on success
//kind of useless for now, but later (TODO) will have it maybe modify the SLL to have things
//like file size, etc in this step rather than in create_dir_SLL, so keeping them separate for now
int list_dir(const char *path, struct String_Linked_List *result)
{
	create_dir_SLL(path, result);
	return 0;
}

//NOTE: kind of defunct for now. Deprecated.
//helper to list_dir()
//caller is responsible for dynamically allocated *result
//negative return indicates error, otherwise return is size of string (bytes)
int pack_SLL_to_CString(struct String_Linked_List *head, char **result)
{
	if (head == NULL) {
		perror("pack_SLL_to_CString - nullPtrError\n");
		return -1;
	}
	size_t string_total_length = 0;
	struct String_Linked_List *position = head;
	position = head;
	while (position->next != NULL) {
		string_total_length += position->length + 1;
		printf("%s\n", position->str); //TODO - remove once GUI printout is done
		position = position->next;
	}
	string_total_length += position->length + 1;

	printf("Debug: total string length: %d\n", string_total_length);
	*result = (char *) malloc(sizeof(char) * string_total_length);
	char *current_pos = *result;
	position = head;
	while (position->next != NULL) {
		strcpy(current_pos, position->str);
		current_pos += position->length + 1;
		position = position->next;
	}
	strcpy(current_pos, position->str);
	current_pos += position->length + 1;
	printf("DEBUG: offset = %d\n", current_pos - *result);

	//should now have one big char array of strings in *result.
	//go through it and replace all the \0 with \n, then add \0 at the very end

	printf("DEBUG - big string\n\n");
	for (size_t handle = 0; handle < string_total_length; ++handle) {
		if ((*result)[handle] == '\0')
			(*result)[handle] = '\n';
		putchar((*result)[handle]);
	}

	return string_total_length;
}
/* //note: commented out, using TableContents below for now
int SELECTED_ELEMENT = 1;
int TableMessage(UIElement *element, UIMessage message, int di, void *dp)
{
	if (message == UI_MSG_TABLE_GET_ITEM) {
		UITableGetItem *item_ptr = (UITableGetItem *) dp;
		item_ptr->isSelected = (item_ptr->index == SELECTED_ELEMENT);
		size_t bytes = snprintf(item_ptr->buffer, item_ptr->bufferBytes, "debug:"); 
		printf("DEBUG - TableMessage\n");
		printf("%d bytes into buffer. Contents: %s\n", bytes, item_ptr->buffer);
		return bytes;
	}
}
*/

//#include <stdio.h> //remove once snprintf is no longer needed
int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp)
{
	if (message == UI_MSG_CLICKED) {
		struct String_Linked_List result;
		uint8_t list_status = list_dir(".", &result);
		printf("Made it past list_dir\n");
		fflush(stdout);
		memcpy(&Global_Dir_Listing, &result, sizeof(struct String_Linked_List));
		printf("Made it past memcpy op\n");
		fflush(stdout);
		//if (list_status != 0) { printf("list_dir returned nonzero in ButtonMessage\n"); } //TODO -- better handle this, once the error code is actually relevant

		UIElementRefresh(&dir_listing->e);
		UIElementRefresh(dir_listing->e.parent);
		/* --was used for debugging -- now commented out
		UILabelSetContent(label, result, listing_bytes);
		UIElementRefresh(&label->e);
		UIElementRefresh(label->e.parent);
		*/
	}
	return 0;
}

int table_selected_element = 1;
int TableContents(UIElement *element, UIMessage message, int di, void *dp)
{
	if (message == UI_MSG_TABLE_GET_ITEM) {
		UITableGetItem *ptr = (UITableGetItem *) dp;
		ptr->isSelected = (table_selected_element == ptr->index);
		struct String_Linked_List *dir_entry = &Global_Dir_Listing;
		for(int i = 0; i < ptr->index; ++i) {
			dir_entry = dir_entry->next;
			if (dir_entry == NULL) break;
		}
		size_t bytes = snprintf(ptr->buffer, ptr->bufferBytes,
			(dir_entry == NULL) ? "NULL" : dir_entry->str);
		printf("%d bytes into buffer. Contents: %s\n", bytes, ptr->buffer);
		return bytes;
	} else if (message == UI_MSG_LEFT_DOWN) {
		int hit = UITableHitTest((UITable *) element, element->window->cursorX, element->window->cursorY);
		if (table_selected_element != hit) {
			table_selected_element = hit;
			if (!UITableEnsureVisible((UITable *) element, table_selected_element)) {
				UIElementRepaint(element, NULL);
			}
		}
	}
	return 0;
}

#define TEXTBOX_ENTER_KEYSTROKE 13
int TextboxMessage(UIElement *element, UIMessage message, int di, void *dp)
{
	if (message == UI_MSG_VALUE_CHANGED) {
	}
	if (message == UI_MSG_KEY_TYPED) {
		UIKeyTyped *ui_key = (UIKeyTyped *) dp;
		if (ui_key->text != NULL) {
			//printf("UI-key-event text: %d\n", *(ui_key->text));
			if (*(ui_key->text) == TEXTBOX_ENTER_KEYSTROKE) {
				UILabelSetContent(label, textbox->string, textbox->bytes);
				UIElementRefresh(&label->e);
				UIElementRefresh(label->e.parent);
			}
		}
				
	}
	return 0;
}
