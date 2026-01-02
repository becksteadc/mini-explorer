#define UI_IMPLEMENTATION
#define UI_WINDOWS
#include "luigi.h"
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

UITextbox *textbox;
UIButton *button;
UILabel *label;
UICode *dir_listing;

struct String_Linked_List {
	char *str;
	size_t length;
	struct String_Linked_List *next;
};


int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp);
int TextboxMessage(UIElement *element, UIMessage message, int di, void *dp);
int list_dir(const char *path, char **result);
int pack_SLL_to_CString(struct String_Linked_List *head, char **result);

int WinMain(HINSTANCE instance, HINSTANCE previousInstance,
			LPSTR commandline, int showCommand)
{
	UIInitialise();
	UIWindow *window = UIWindowCreate(0, 0, "hello", 640, 480);

	UIPanel *panel = UIPanelCreate(&window->e, UI_PANEL_GRAY |
									UI_PANEL_MEDIUM_SPACING);
	
	button = UIButtonCreate(&panel->e, 0, "Push", -1);
	textbox = UITextboxCreate(&panel->e, 0);
	label = UILabelCreate(&panel->e, 0, "text", -1);
	dir_listing = UICodeCreate(&panel->e, 0); 

	button->e.messageUser = ButtonMessage;
	textbox->e.messageUser = TextboxMessage;

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

//caller must free returned result
int list_dir(const char *path, char **result)
{
	struct dirent *entry;
	DIR *dp;
	dp = opendir(path);
	if (dp == NULL) {
		perror("list-dir: Unable to open directory.");
		return -1;
	}
	#define MAX_FNAME_LENGTH 256
	char buf[MAX_FNAME_LENGTH];
	struct String_Linked_List *names = NULL;
	struct String_Linked_List *end = NULL;
	while ((entry = readdir(dp))) {
		//save each entry string
		//get all their cumulative lengths
		//combine into one malloc'd block
		//free up the temporary holdings for those strings
		
		//set up the head of the linked list
		if (names == NULL) {
			names = (struct String_Linked_List *) malloc(sizeof(struct String_Linked_List)); //MALLOC --
			names->length = strlen(entry->d_name);
			names->str = (char *) malloc(sizeof(char) * names->length + 1); //MALLOC --
			strcpy(names->str, entry->d_name);
			names->next = NULL;
			end = names;
		} else {
			struct String_Linked_List *new = malloc(sizeof(struct String_Linked_List)); //MALLOC --
			end->next = new;
			new->length = strlen(entry->d_name);
			new->str = (char *) malloc(sizeof(char) * new->length + 1); //MALLOC --
			strcpy(new->str, entry->d_name);
			new->next = NULL;

			end = new; //defer'd / must be at end of else block
		}
	}
	
	if (pack_SLL_to_CString(names, result)) {
		printf("Panic -- pack_SLL_to_CString received null pointer\n");
		exit(-1);
	}
	cleanup_SLL(names);

	return 0;
}

//helper to list_dir()
//caller is responsible for dynamically allocated *result
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

	printf("DEBUG: returning zero\n");
	return 0;
}

//#include <stdio.h> //remove once snprintf is no longer needed
int ButtonMessage(UIElement *element, UIMessage message, int di, void *dp)
{
	if (message == UI_MSG_CLICKED) {
		//TODO finish implementing properly
		char *result;
		list_dir(".", &result);
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
