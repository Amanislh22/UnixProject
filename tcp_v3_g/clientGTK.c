#include <gtk/gtk.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include "structures.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *username_entry;
    GtkWidget *password_entry;
    GtkWidget *login_box;
    GtkWidget *main_box;
    GtkWidget *result_text;
    GtkWidget *file_entry;
    int sockfd;
    char buffer[MAX_BUFFER];
} AppData;

typedef struct {
    AppData *app;
    int service_type;
    char *filename;
} ServiceData;

// Prototypes
void show_error_dialog(const char *message);
void show_main_interface(AppData *app);
gboolean handle_service_response(gpointer user_data);
void on_connect_clicked(GtkButton *button, AppData *app);
void on_service_clicked(GtkButton *button, gpointer user_data);
gboolean handle_service_async(gpointer user_data);

void show_error_dialog(const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
                                             GTK_DIALOG_MODAL,
                                             GTK_MESSAGE_ERROR,
                                             GTK_BUTTONS_OK,
                                             "%s", message);
    g_signal_connect_swapped(dialog, "response",
                           G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show(dialog);
}

gboolean handle_service_async(gpointer user_data) {
    ServiceData *data = (ServiceData *)user_data;
    AppData *app = data->app;
    
    if (send(app->sockfd, &data->service_type, sizeof(int), 0) < 0) {
        show_error_dialog("Erreur d'envoi de la commande");
        g_free(data);
        return FALSE;
    }

    if (data->service_type == 3 && data->filename) {
        if (send(app->sockfd, data->filename, strlen(data->filename) + 1, 0) < 0) {
            show_error_dialog("Erreur d'envoi du nom de fichier");
            g_free(data);
            return FALSE;
        }
    }

    g_idle_add(handle_service_response, app);
    
    if (data->filename) {
        g_free(data->filename);
    }
    g_free(data);
    
    return FALSE;
}

gboolean handle_service_response(gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->result_text));
    char resp[MAX_BUFFER];
    ssize_t bytes_received;
    
    int flags = fcntl(app->sockfd, F_GETFL, 0);
    fcntl(app->sockfd, F_SETFL, flags | O_NONBLOCK);

    bytes_received = recv(app->sockfd, resp, MAX_BUFFER - 1, 0);
    
    if (bytes_received > 0) {
        resp[bytes_received] = '\0';
        
        if (strcmp(resp, "END") == 0) {
            fcntl(app->sockfd, F_SETFL, flags);
            return FALSE;
        }
        
        GtkTextIter end;
        gtk_text_buffer_get_end_iter(buffer, &end);
        gtk_text_buffer_insert(buffer, &end, resp, -1);
        gtk_text_buffer_insert(buffer, &end, "\n", -1);
        
        return TRUE;
    } else if (bytes_received == 0) {
        show_error_dialog("Connexion fermée par le serveur");
        fcntl(app->sockfd, F_SETFL, flags);
        return FALSE;
    } else if (bytes_received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            show_error_dialog("Erreur de réception des données");
            fcntl(app->sockfd, F_SETFL, flags);
            return FALSE;
        }
        return TRUE;
    }

    return TRUE;
}

void on_connect_clicked(GtkButton *button, AppData *app) {
    const char *username = gtk_entry_get_text(GTK_ENTRY(app->username_entry));
    const char *password = gtk_entry_get_text(GTK_ENTRY(app->password_entry));

    Auth_Data auth;
    memset(&auth, 0, sizeof(auth));
    strncpy(auth.username, username, MAX_USERNAME - 1);
    strncpy(auth.password, password, MAX_PASSWORD - 1);

    if (send(app->sockfd, &auth, sizeof(auth), 0) < 0) {
        show_error_dialog("Erreur d'envoi des informations d'authentification");
        return;
    }

    char response[MAX_BUFFER];
    memset(response, 0, MAX_BUFFER);
    
    if (recv(app->sockfd, response, MAX_BUFFER - 1, 0) <= 0) {
        show_error_dialog("Erreur de réception de la réponse du serveur");
        return;
    }

    if (strcmp(response, "OK") == 0) {
        GList *children, *iter;
        children = gtk_container_get_children(GTK_CONTAINER(app->window));
        for(iter = children; iter != NULL; iter = g_list_next(iter))
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        g_list_free(children);
        
        app->login_box = NULL;
        show_main_interface(app);
        gtk_widget_show_all(app->window);
    } else {
        show_error_dialog("Authentification échouée");
    }
}

void on_service_clicked(GtkButton *button, gpointer user_data) {
    ServiceData *data = g_new(ServiceData, 1);
    AppData *app = (AppData *)user_data;
    
    const gchar *button_label = gtk_button_get_label(button);
    data->app = app;
    data->filename = NULL;

    if (strcmp(button_label, "Date/Heure") == 0)
        data->service_type = 1;
    else if (strcmp(button_label, "Liste Fichiers") == 0)
        data->service_type = 2;
    else if (strcmp(button_label, "Contenu Fichier") == 0) {
        data->service_type = 3;
        const char *filename = gtk_entry_get_text(GTK_ENTRY(app->file_entry));
        if (strlen(filename) > 0) {
            data->filename = g_strdup(filename);
        } else {
            show_error_dialog("Veuillez entrer un nom de fichier");
            g_free(data);
            return;
        }
    }
    else if (strcmp(button_label, "Durée Connexion") == 0)
        data->service_type = 4;
    else if (strcmp(button_label, "Quitter") == 0) {
        data->service_type = 5;
        int choice = 5;
        send(app->sockfd, &choice, sizeof(choice), 0);
        gtk_main_quit();
        g_free(data);
        return;
    }

    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->result_text));
    gtk_text_buffer_set_text(buffer, "", 0);
    
    g_idle_add(handle_service_async, data);
}

void show_main_interface(AppData *app) {
    app->main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app->window), app->main_box);

    gtk_widget_set_margin_start(app->main_box, 10);
    gtk_widget_set_margin_end(app->main_box, 10);
    gtk_widget_set_margin_top(app->main_box, 10);
    gtk_widget_set_margin_bottom(app->main_box, 10);

    // Zone de résultat
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scroll, 400, 300);
    app->result_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->result_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->result_text), GTK_WRAP_WORD_CHAR);
    gtk_container_add(GTK_CONTAINER(scroll), app->result_text);
    gtk_box_pack_start(GTK_BOX(app->main_box), scroll, TRUE, TRUE, 0);

    // Zone de saisie du nom de fichier
    GtkWidget *file_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *file_label = gtk_label_new("Nom du fichier:");
    app->file_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(file_box), file_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(file_box), app->file_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(app->main_box), file_box, FALSE, FALSE, 5);

    // Boutons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *date_button = gtk_button_new_with_label("Date/Heure");
    GtkWidget *list_button = gtk_button_new_with_label("Liste Fichiers");
    GtkWidget *content_button = gtk_button_new_with_label("Contenu Fichier");
    GtkWidget *time_button = gtk_button_new_with_label("Durée Connexion");
    GtkWidget *quit_button = gtk_button_new_with_label("Quitter");

    g_signal_connect(date_button, "clicked", G_CALLBACK(on_service_clicked), app);
    g_signal_connect(list_button, "clicked", G_CALLBACK(on_service_clicked), app);
    g_signal_connect(content_button, "clicked", G_CALLBACK(on_service_clicked), app);
    g_signal_connect(time_button, "clicked", G_CALLBACK(on_service_clicked), app);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_service_clicked), app);

    gtk_box_pack_start(GTK_BOX(button_box), date_button, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(button_box), list_button, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(button_box), content_button, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(button_box), time_button, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(button_box), quit_button, TRUE, TRUE, 2);
    
    gtk_box_pack_start(GTK_BOX(app->main_box), button_box, FALSE, FALSE, 5);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <ip_serveur> <port>\n", argv[0]);
        return 1;
    }

    AppData *app = g_new0(AppData, 1);
    app->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (app->sockfd < 0) {
        perror("Erreur création socket");
        g_free(app);
        return 1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        fprintf(stderr, "Adresse IP invalide\n");
        close(app->sockfd);
        g_free(app);
        return 1;
    }

    if (connect(app->sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erreur connexion");
        close(app->sockfd);
        g_free(app);
        return 1;
    }

    gtk_init(&argc, &argv);

    // Création de la fenêtre principale
    app->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(app->window), "Client TCP");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 500, 400);
    g_signal_connect(app->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Interface de connexion
    app->login_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app->window), app->login_box);
    
    gtk_widget_set_margin_start(app->login_box, 10);
    gtk_widget_set_margin_end(app->login_box, 10);
    gtk_widget_set_margin_top(app->login_box, 10);
    gtk_widget_set_margin_bottom(app->login_box, 10);
    
    GtkWidget *username_label = gtk_label_new("Nom d'utilisateur:");
    app->username_entry = gtk_entry_new();
    GtkWidget *password_label = gtk_label_new("Mot de passe:");
    app->password_entry = gtk_entry_new();
    gtk_entry_set_visibility(GTK_ENTRY(app->password_entry), FALSE);
    GtkWidget *connect_button = gtk_button_new_with_label("Se connecter");

    gtk_box_pack_start(GTK_BOX(app->login_box), username_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app->login_box), app->username_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app->login_box), password_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app->login_box), app->password_entry, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(app->login_box), connect_button, FALSE, FALSE, 0);

   g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_clicked), app);

    gtk_widget_show_all(app->window);
    gtk_main();

    if (app->sockfd >= 0) {
        close(app->sockfd);
    }
    g_free(app);
    return 0;
}