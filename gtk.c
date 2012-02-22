#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <wait.h>

GtkTextBuffer* buffer;
gchar* composer_content;
gchar* composer_title;

void debugf( char *string, ... ) __attribute__ ( ( format( printf, 1, 2 ) ) );

/* Callback functions for the window */
static gboolean delete_event (GtkWidget *widget,
		GdkEvent *event, gpointer data)
{
	if (composer_content != NULL) {
		free(composer_content);
	}
	gtk_main_quit();
	return FALSE;
}

/* Callback for saving the data */
static void send_to_composer (GtkWidget *widget,
		GdkEvent *event, gpointer data)
{
	GtkTextIter start;
	GtkTextIter end;

	gchar *text;

	/* Obtain iters for the start and end of points of the buffer */
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);

	/* Get the entire buffer text. */
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

	void send_gmcp_composer_content (char *text);
	send_gmcp_composer_content (text);
	free(text);
}

static void load_from_composer (GtkWidget *widget,
		GdkEvent *event, gpointer data)
{
	GtkTextIter start;
	GtkTextIter end;

	if (composer_content == NULL) {
		return;
	}

	/* Obtain iters for the start and end of points of the buffer */
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);

	/* Clear the entire buffer */
	gtk_text_buffer_delete (buffer, &start, &end);

	gtk_text_buffer_insert (buffer, &start, composer_content, 
			strlen(composer_content));
}

void gtk_show_editor( )
{
	pid_t child_pid;
	pid_t wait_pid;
	int child_status;

	/* We don't want our window to freeze mudbot
	 * so we fork the window process */
	child_pid = fork();
	if (child_pid == -1) {
		debugf("fork: Editor fork failed");
		return;
	} else if (child_pid != 0) {

		/* This is the parent process.
		 * wait for child to signal exit 
		 * before going on*/
	
		do {
			wait_pid = wait(&child_status);
		} while (wait_pid != child_pid);

		return;
	}

	/*
	 * Child process emidiately forks a new child and exits.
	 * This is a simple zombie process prevention.
	 */
	child_pid = fork();
	if (child_pid == -1) {
		debugf("fork: Editor fork failed");
		_exit(1);
	} else if (child_pid != 0) {
		/* This is the parent, so we die */
		_exit(0);
	}

	/* The child of the child process keeps on truckin' */

	debugf("fork: Forked process for editor (pid: %d)", (int) getpid());
	gtk_init (NULL, NULL);

	/* In child process */
	GtkWidget* window;
	GtkWidget* text;
	GtkWidget* scrollwindow;
	GtkWidget* table;
	GtkWidget* separator;
	GtkWidget* send_button;
	GtkWidget* load_button;

	/* Create our window with signals */
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	g_signal_connect (window, "delete-event",
			G_CALLBACK (delete_event), NULL);

	/* Create the table and add it to window.
	 * Set the title if we have one. */
	table = gtk_table_new (3, 2, FALSE);
	gtk_container_add (GTK_CONTAINER (window), table);
	if(composer_title != NULL) {
		gtk_window_set_title (GTK_WINDOW (window), composer_title);
	}
	gtk_widget_show (table);

	/* Create the buff, clear it if empty */
	buffer = gtk_text_buffer_new (NULL);

	/* Create a textarea with content if existent.
	 * Add it to a scrolled window and
	 * then add it to table. */
	scrollwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwindow), 
			GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request (scrollwindow, 500, 500);
	gtk_widget_show (scrollwindow);

	if(composer_content != NULL) {
		gtk_text_buffer_insert_at_cursor (buffer, composer_content, strlen(composer_content));
	}
	text = gtk_text_view_new_with_buffer (buffer);
	gtk_widget_set_size_request (text, 500, 500);
	gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(text), GTK_WRAP_WORD);
	gtk_widget_show (text);
	gtk_container_add (GTK_CONTAINER (scrollwindow), text);

	gtk_table_attach_defaults (GTK_TABLE (table), scrollwindow, 0, 2, 0, 1);

	/* Create separator and ad it to table */
	separator = gtk_hseparator_new();
	gtk_table_attach_defaults (GTK_TABLE (table), separator, 0, 2, 1, 2);
	gtk_widget_show (separator);

	/* Create send_button with signal-handler and add it to the table */
	send_button = gtk_button_new_with_label ("Send to composer");
	g_signal_connect (send_button, "clicked",
			G_CALLBACK (send_to_composer), NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), send_button, 0, 1, 2, 3);
	gtk_widget_show (send_button);

	/* Create load_button with signal handler and add to table */
	load_button = gtk_button_new_with_label ("Reload composer buffer");
	g_signal_connect (load_button, "clicked",
			G_CALLBACK (load_from_composer), NULL);
	gtk_table_attach_defaults (GTK_TABLE (table), load_button, 1, 2, 2, 3);
	gtk_widget_set_sensitive(load_button, composer_content != NULL);
	gtk_widget_show (load_button);

	/* Show and start widget */
	gtk_widget_show (window);
	gtk_main ();

	/* Child process is done so we exit */
	_exit(0);
}

void gtk_set_composer_content (char* content)
{
	int content_len;
	char *p;

	/* Replace \n string for newlines */
	p = strstr(content, "\\n");
	while (p != NULL) {
		memcpy(p, "\r\n", 2);
		p = strstr(content, "\\n");
	}

	/* Clear out the old buffer */
	if (composer_content != NULL) {
		free(composer_content);
		composer_content = NULL;
	}

	content_len = strlen (content);
	composer_content = calloc (content_len, sizeof(char));
	strcpy(composer_content, content);
}

void gtk_set_composer_title (char* title)
{
	int title_len;

	/* Clear out the old buffer */
	if (composer_title != NULL) {
		free(composer_title);
		composer_title = NULL;
	}

	title_len = strlen (title);
	composer_title = calloc (title_len, sizeof(char));
	strcpy(composer_title, title);
}
