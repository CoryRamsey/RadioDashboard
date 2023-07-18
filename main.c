#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include "radio_control.h"
#include <glib.h>
#include <hamlib/rig.h>

#define UPDATE_INTERVAL_MS 50

extern bool is_connected;

// widgets
typedef struct {
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *connected_icon;
    GtkWidget *disconnected_icon;
    bool connected;
} WidgetData;

// frequency label update
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

//  "Connect" button Callback
void on_connect_button_clicked(GtkWidget *button, WidgetData *widget_data) {
    if (connect_radio()) {
        gtk_label_set_text(GTK_LABEL(widget_data->label), "Radio connected");
        widget_data->connected = true;
        gtk_widget_show(widget_data->connected_icon);
        gtk_widget_hide(widget_data->disconnected_icon);
        update_frequency_label(widget_data); // Update the frequency label
    } else {
        gtk_label_set_text(GTK_LABEL(widget_data->label), "Failed to connect");
    }
}

// "Disconnect" button Callback
void on_disconnect_button_clicked(GtkWidget *button, WidgetData *widget_data) {
    disconnect_radio();
    gtk_label_set_text(GTK_LABEL(widget_data->label), "Radio disconnected");
    widget_data->connected = false;
    gtk_widget_show(widget_data->disconnected_icon);
    gtk_widget_hide(widget_data->connected_icon);
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

// Destroy the GTK application when the window is closed
void on_window_destroy(GtkWidget *widget, gpointer data) {
    disconnect_radio(); // Ensure the radio is disconnected before closing
    gtk_main_quit();
}

int main(int argc, char *argv[]) {
    // Initialize GTK
    gtk_init(&argc, &argv);

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

    // connect button
    GtkWidget *connect_button = gtk_button_new_with_label("Connect");
    gtk_grid_attach(GTK_GRID(grid), connect_button, 0, 2, 1, 1);
    g_signal_connect(connect_button, "clicked", G_CALLBACK(on_connect_button_clicked), &widget_data);

    // disconnect button
    GtkWidget *disconnect_button = gtk_button_new_with_label("Disconnect");
    gtk_grid_attach(GTK_GRID(grid), disconnect_button, 1, 2, 1, 1);
    g_signal_connect(disconnect_button, "clicked", G_CALLBACK(on_disconnect_button_clicked), &widget_data);

    // icons
    widget_data.connected_icon = gtk_image_new_from_icon_name("gtk-yes", GTK_ICON_SIZE_BUTTON);
    widget_data.disconnected_icon = gtk_image_new_from_icon_name("gtk-no", GTK_ICON_SIZE_BUTTON);

    // Hide the disconnected icon initially
    gtk_widget_hide(widget_data.disconnected_icon);

    // Add the icons to the grid
    gtk_grid_attach(GTK_GRID(grid), widget_data.connected_icon, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), widget_data.disconnected_icon, 2, 1, 1, 1);

    // try to connect
    if (!connect_radio()) {
        gtk_label_set_text(GTK_LABEL(widget_data.label), "Failed to connect to the radio.");
    } else {
        gtk_label_set_text(GTK_LABEL(widget_data.label), "Radio connected");
        widget_data.connected = true;
        gtk_widget_show(widget_data.connected_icon);
    }

    // Connect the destroy signal to close the application properly
    g_signal_connect(window, "destroy", G_CALLBACK(on_window_destroy), NULL);

    // Update Frequency
    g_timeout_add(UPDATE_INTERVAL_MS, update_frequency_label, &widget_data);

    // Show all the widgets
    gtk_widget_show_all(window);

    // gtk loop
    gtk_main();

    // Disconnect from the radio before exiting
    disconnect_radio();

    return 0;
}
