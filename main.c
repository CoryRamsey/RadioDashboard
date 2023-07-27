#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "record.h"
#include "radio_control.h"

#define UPDATE_INTERVAL_MS 50

extern bool is_connected;
extern bool recording_in_progress;

void stop_recording_thread(gpointer data);
gboolean on_record_timeout(gpointer data);

// Widgets
typedef struct {
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *connected_icon;
    GtkWidget *disconnected_icon;
    GtkWidget *record_icon; 
    GtkWidget *connect_button; 
    GtkWidget *record_button; 
    bool connected;
} WidgetData;

// Function declarations
void stop_recording_thread(gpointer data);
gboolean on_record_timeout(gpointer data);
gboolean update_frequency_label(gpointer data);
void on_connect_button_clicked(GtkWidget *button, WidgetData *widget_data);
void on_set_frequency_button_clicked(GtkWidget *button, WidgetData *widget_data);
gpointer record_thread_func(gpointer data);
void on_record_button_clicked(GtkWidget *button, WidgetData *widget_data);
gboolean idle_hide_record_icon(gpointer data);

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

    recording_in_progress = false;

    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;

    // main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Radio Control");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_container_add(GTK_CONTAINER(window), grid);

    // frequency label
    label = gtk_label_new("Frequency: N/A");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 3, 1);

    // frequency input
    entry = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

    // WidgetData to store references to label, entry, and connection status
    WidgetData widget_data;
    widget_data.label = label;
    widget_data.entry = entry;
    widget_data.connected = false;

    // set frequency button
    GtkWidget *set_frequency_button = gtk_button_new_with_label("Set Frequency (MHz)");
    gtk_grid_attach(GTK_GRID(grid), set_frequency_button, 1, 1, 1, 1);
    g_signal_connect(set_frequency_button, "clicked", G_CALLBACK(on_set_frequency_button_clicked), &widget_data);

    // Combine "Connect" and "Disconnect" buttons
    widget_data.connect_button = gtk_button_new_with_label("Connect");
    gtk_grid_attach(GTK_GRID(grid), widget_data.connect_button, 0, 2, 1, 1);
    g_signal_connect(widget_data.connect_button, "clicked", G_CALLBACK(on_connect_button_clicked), &widget_data);

    // icons
    widget_data.connected_icon = gtk_image_new_from_icon_name("gtk-yes", GTK_ICON_SIZE_BUTTON);
    widget_data.disconnected_icon = gtk_image_new_from_icon_name("gtk-no", GTK_ICON_SIZE_BUTTON);

    // Hide the disconnected icon initially
    gtk_widget_hide(widget_data.disconnected_icon);

    // Add the icons to the grid
    gtk_grid_attach(GTK_GRID(grid), widget_data.connected_icon, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widget_data.disconnected_icon, 1, 2, 1, 1);

    // recording indication icon
    widget_data.record_icon = gtk_image_new_from_icon_name("media-record", GTK_ICON_SIZE_BUTTON);
    gtk_widget_hide(widget_data.record_icon); // Hide the icon initially
    gtk_grid_attach(GTK_GRID(grid), widget_data.record_icon, 1, 3, 1, 1);

    // Connect the destroy signal to close the application properly
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Delay the visibility of the recording icon to ensure it's hidden at startup
    g_idle_add(idle_hide_record_icon, &widget_data);

    // record button
    widget_data.record_button = gtk_button_new_with_label("Record");
    gtk_grid_attach(GTK_GRID(grid), widget_data.record_button, 0, 3, 1, 1);
    g_signal_connect(widget_data.record_button, "clicked", G_CALLBACK(on_record_button_clicked), &widget_data);

    // Show all the widgets
    gtk_widget_show_all(window);

    // gtk loop
    gtk_main();

    return 0;
}

// Frequency label update
gboolean update_frequency_label(gpointer data) {
    WidgetData *widget_data = (WidgetData *)data;
    GtkWidget *label = widget_data->label;

    if (!widget_data->connected) {
        gtk_label_set_text(GTK_LABEL(label), "Radio not connected");
        return G_SOURCE_CONTINUE;
    }

    double currentFrequency = get_current_frequency();
    char freq_text[64];
    snprintf(freq_text, sizeof(freq_text), "Current frequency: %.6f MHz", currentFrequency);
    gtk_label_set_text(GTK_LABEL(label), freq_text);

    // Returning TRUE allows the timeout function to continue running periodically.
    return G_SOURCE_CONTINUE;
}

// "Connect/Disconnect" button Callback
void on_connect_button_clicked(GtkWidget *button, WidgetData *widget_data) {
    if (!widget_data->connected) {
        // Connect radio
        if (connect_radio()) {
            gtk_label_set_text(GTK_LABEL(widget_data->label), "Radio connected");
            widget_data->connected = true;
            gtk_widget_show(widget_data->connected_icon);
            gtk_widget_hide(widget_data->disconnected_icon);
            gtk_button_set_label(GTK_BUTTON(widget_data->connect_button), "Disconnect");
            update_frequency_label(widget_data); // Update the frequency label
        } else {
            gtk_label_set_text(GTK_LABEL(widget_data->label), "Failed to connect");
        }
    } else {
        // Disconnect radio
        disconnect_radio();
        gtk_label_set_text(GTK_LABEL(widget_data->label), "Radio disconnected");
        widget_data->connected = false;
        gtk_widget_show(widget_data->disconnected_icon);
        gtk_widget_hide(widget_data->connected_icon);
        gtk_button_set_label(GTK_BUTTON(widget_data->connect_button), "Connect");
    }
}

// "Set Frequency" button Callback
void on_set_frequency_button_clicked(GtkWidget *button, WidgetData *widget_data) {
    const char *freq_str = gtk_entry_get_text(GTK_ENTRY(widget_data->entry));
    double frequency_mhz = atof(freq_str); // Convert the input string to a double

    // Check if the conversion was successful and the input is a valid number
    if (frequency_mhz <= 0) { //TODO Better Validation
        gtk_label_set_text(GTK_LABEL(widget_data->label), "Invalid frequency");
        return;
    }

    // Convert MHz to Hz
    freq_t frequency_hz = (freq_t)(frequency_mhz * 1000000.0);

    if (set_freq(frequency_hz)) {
        char freq_text[64];
        snprintf(freq_text, sizeof(freq_text), "Current frequency: %.9f MHz", frequency_mhz);
        gtk_label_set_text(GTK_LABEL(widget_data->label), freq_text);
    } else {
        gtk_label_set_text(GTK_LABEL(widget_data->label), "Failed to set frequency");
    }
}

// Function to stop the recording process and update the button label
void stop_recording_thread(gpointer data) {
    WidgetData *widget_data = (WidgetData *)data;
    recording_in_progress = false;
    gtk_button_set_label(GTK_BUTTON(widget_data->record_button), "File Saved"); // Change the button label to "File Saved"

    // Delay the change back to "Record" label using a timeout
    g_timeout_add(1000, (GSourceFunc)on_record_timeout, widget_data);
}

// Timeout function to switch the button label back to "Record"
gboolean on_record_timeout(gpointer data) {
    WidgetData *widget_data = (WidgetData *)data;
    gtk_button_set_label(GTK_BUTTON(widget_data->record_button), "Record");
    return G_SOURCE_REMOVE;
}

// Function to run the recording process in a separate thread
gpointer record_thread_func(gpointer data) {
    record();
    return NULL;
}

// "Record" button Callback
void on_record_button_clicked(GtkWidget *button, WidgetData *widget_data) {
    if (!recording_in_progress) {
        // Start recording
        recording_in_progress = true;
        widget_data->record_button = button; // Store the record button widget for later use
        g_thread_new("Recording Thread", record_thread_func, widget_data);
        gtk_widget_show(widget_data->record_icon); // Show the recording indication icon
        gtk_button_set_label(GTK_BUTTON(button), "Stop Recording"); // Change the button label
    } else {
        // Stop recording
        stop_recording_thread(widget_data);
        gtk_widget_hide(widget_data->record_icon); // Hide the recording indication icon
    }
}

// Delay the visibility of the recording icon to ensure it's hidden at startup
gboolean idle_hide_record_icon(gpointer data) {
    WidgetData *widget_data = (WidgetData *)data;
    gtk_widget_hide(widget_data->record_icon);
    return G_SOURCE_REMOVE;
}
