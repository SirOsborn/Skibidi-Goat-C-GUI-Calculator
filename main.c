#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_DISPLAY 50
#define HISTORY_SIZE 5
#define MAX_NUMBERS 100
#define MAX_OPERATORS 100

// Custom colors in RGBA format
#define COLOR_BG "#1e1e2eff"
#define COLOR_DISPLAY "#313244ff"
#define COLOR_NUM_BTN "#45475aee"
#define COLOR_OP_BTN "#89b4faee"
#define COLOR_FUNC_BTN "#f38ba8ee"
#define COLOR_EQ_BTN "#a6e3a1ee"
#define COLOR_TEXT "#cdd6f4ff"
#define COLOR_DISPLAY_TEXT "#f5e0dcff"

// Button styles
#define BUTTON_CSS "button { \
    background: %s; \
    color: " COLOR_TEXT "; \
    font-size: 20px; \
    font-weight: 500; \
    border-radius: 16px; \
    margin: 5px; \
    padding: 8px; \
    box-shadow: 0 2px 4px rgba(0, 0, 0, 0.2); \
    border: none; \
    transition: all 200ms ease; \
} \
button:hover { \
    filter: brightness(110%%); \
    transform: translateY(-1px); \
    box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3); \
} \
button:active { \
    transform: translateY(1px); \
    box-shadow: 0 1px 2px rgba(0, 0, 0, 0.2); \
}"

// Window style
#define WINDOW_CSS "window { \
    background: linear-gradient(145deg, " COLOR_BG ", shade(" COLOR_BG ", 0.9)); \
    border-radius: 24px; \
}"

// Label styles
#define HISTORY_CSS "label { \
    color: " COLOR_TEXT "; \
    font-size: 14px; \
    font-weight: 400; \
    margin: 4px; \
}"

#define DISPLAY_CSS "label { \
    color: " COLOR_DISPLAY_TEXT "; \
    font-size: 48px; \
    font-weight: 600; \
    font-family: 'FiraCode', nerd; \
    margin: 4px; \
    text-shadow: 0 2px 4px rgba(0, 0, 0, 0.1); \
}"

// Global variables
static char display[MAX_DISPLAY] = "0";
static char history[HISTORY_SIZE][4];
static int history_count = 0;
static int clear_next = 0;
static char current_expression[MAX_DISPLAY] = "";

// GTK widgets
static GtkWidget *display_label;
static GtkWidget *mini_display_label;
static GtkWidget *history_label;

// Helper function to check if a character is an operator
static int is_operator(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || c == '×' || c == '÷';
}

// Helper function to get operator precedence
static int get_precedence(char op) {
    if (op == '*' || op == '/' || op == '×' || op == '÷') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

// Helper function to perform operation
static double apply_operator(double a, double b, char op) {
    switch(op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': case '×': return a * b;
        case '/': case '÷': return b != 0 ? a / b : 0;
        default: return 0;
    }
}

// Function to evaluate expression with operator precedence
static double evaluate_expression(const char* expr) {
    double numbers[MAX_NUMBERS];
    char operators[MAX_OPERATORS];
    int num_count = 0;
    int op_count = 0;
    char number[MAX_DISPLAY] = "";
    int num_len = 0;

    for (size_t i = 0; expr[i] != '\0'; i++) {
        char c = expr[i];
        if (isdigit(c) || c == '.') {
            number[num_len++] = c;
            number[num_len] = '\0';
        } else if (is_operator(c)) {
            if (num_len > 0) {
                numbers[num_count++] = atof(number);
                num_len = 0;
                number[0] = '\0';
            }
            if (c == '-' && (i == 0 || is_operator(expr[i-1]))) {
                number[num_len++] = '-';
                number[num_len] = '\0';
                continue;
            }

            while (op_count > 0 && get_precedence(operators[op_count-1]) >= get_precedence(c)) {
                double b = numbers[--num_count];
                double a = numbers[--num_count];
                char op = operators[--op_count];
                numbers[num_count++] = apply_operator(a, b, op);
            }
            operators[op_count++] = c;
        }
    }

    if (num_len > 0) {
        numbers[num_count++] = atof(number);
    }

    while (op_count > 0) {
        double b = numbers[--num_count];
        double a = numbers[--num_count];
        char op = operators[--op_count];
        numbers[num_count++] = apply_operator(a, b, op);
    }

    return num_count > 0 ? numbers[0] : 0;
}

static void update_history(const char* calculation) {
    if (history_count >= HISTORY_SIZE) {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        history_count = HISTORY_SIZE - 1;
    }
    strcpy(history[history_count++], calculation);

    char all_history[256] = "";
    for (int i = 0; i < history_count; i++) {
        strcat(all_history, history[i]);
        strcat(all_history, "\n");
    }
    gtk_label_set_text(GTK_LABEL(history_label), all_history);
}

static void update_display_text(void) {
    gtk_label_set_text(GTK_LABEL(display_label), display);
}

static void button_clicked(GtkWidget *widget, gpointer data) {
    const char* label = gtk_button_get_label(GTK_BUTTON(widget));
    char calculation[MAX_DISPLAY * 2];

    if (strcmp(label, "C") == 0) {
        strcpy(display, "0");
        current_expression[0] = '\0';
        clear_next = 0;
        gtk_label_set_text(GTK_LABEL(mini_display_label), "");
    }
    else if (strcmp(label, "±") == 0) {
        if (strlen(display) > 0 && display[0] == '-') {
            memmove(display, display + 1, strlen(display));
        } else {
            memmove(display + 1, display, strlen(display) + 1);
            display[0] = '-';
        }
        strcpy(current_expression, display);
    }
    else if (strcmp(label, "%") == 0) {
        double current = atof(display);
        snprintf(display, MAX_DISPLAY, "%.6g", current / 100.0);
        strcpy(current_expression, display);
    }
    else if (strcmp(label, "=") == 0) {
        if (strlen(current_expression) > 0) {
            double result = evaluate_expression(current_expression);
            snprintf(calculation, sizeof(calculation), "%s = %.6g",
                    current_expression, result);
            update_history(calculation);
            snprintf(display, MAX_DISPLAY, "%.6g", result);
            clear_next = 1;
            gtk_label_set_text(GTK_LABEL(mini_display_label), "");
        }
    }
    else if (is_operator(label[0])) {
        char op = (label[0] == '×') ? '*' : (label[0] == '÷') ? '/' : label[0];
        if (strlen(current_expression) > 0 && !is_operator(current_expression[strlen(current_expression)-1])) {
            size_t len = strlen(current_expression);
            current_expression[len] = op;
            current_expression[len + 1] = '\0';
            strcat(display, label);
        }
    }
    else {
        if (clear_next) {
            strcpy(display, label);
            clear_next = 0;
            current_expression[0] = '\0';
        } else {
            if (strcmp(display, "0") == 0 && strcmp(label, ".") != 0) {
                strcpy(display, label);
            } else if (strlen(display) < MAX_DISPLAY - 1) {
                if (!(strchr(display, '.') && strcmp(label, ".") == 0)) {
                    strcat(display, label);
                }
            }
        }
        strcpy(current_expression, display);
    }

    update_display_text();
}

static GtkWidget* create_button(const char* label, const char* color) {
    GtkWidget *button = gtk_button_new_with_label(label);
    gtk_widget_set_size_request(button, 75, 75);

    // Apply CSS styling
    GtkCssProvider *provider = gtk_css_provider_new();
    char css[1024];
    snprintf(css, sizeof(css), BUTTON_CSS, color);
    gtk_css_provider_load_from_data(provider, css, -1);
    gtk_style_context_add_provider(gtk_widget_get_style_context(button),
                                 GTK_STYLE_PROVIDER(provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);

    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), NULL);
    return button;
}

static void activate(GtkApplication *app, gpointer user_data) {
    // Create main window
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Skibidi Goat Calculator Pro Max");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 650);

    // Apply window styling
    GtkCssProvider *window_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(window_provider, WINDOW_CSS, -1);
    gtk_style_context_add_provider(gtk_widget_get_style_context(window),
                                 GTK_STYLE_PROVIDER(window_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(window_provider);

    // Create main vertical box with padding
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_start(vbox, 20);
    gtk_widget_set_margin_end(vbox, 20);
    gtk_widget_set_margin_top(vbox, 20);
    gtk_widget_set_margin_bottom(vbox, 20);
    gtk_window_set_child(GTK_WINDOW(window), vbox);

    // Create history display
    history_label = gtk_label_new("");
    gtk_widget_set_margin_bottom(history_label, 10);
    gtk_widget_set_halign(history_label, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), history_label);

    // Apply styling to history display
    GtkCssProvider *history_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(history_provider, HISTORY_CSS, -1);
    gtk_style_context_add_provider(gtk_widget_get_style_context(history_label),
                                 GTK_STYLE_PROVIDER(history_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(history_provider);

    // Create mini display
    mini_display_label = gtk_label_new("");
    gtk_widget_set_margin_bottom(mini_display_label, 5);
    gtk_widget_set_halign(mini_display_label, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), mini_display_label);

    // Create main display
    display_label = gtk_label_new("0");
    gtk_widget_set_margin_bottom(display_label, 30);
    gtk_widget_set_halign(display_label, GTK_ALIGN_END);
    gtk_box_append(GTK_BOX(vbox), display_label);

    // Apply styling to displays
    GtkCssProvider *display_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(display_provider, DISPLAY_CSS, -1);
    gtk_style_context_add_provider(gtk_widget_get_style_context(display_label),
                                 GTK_STYLE_PROVIDER(display_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    gtk_style_context_add_provider(gtk_widget_get_style_context(mini_display_label),
                                 GTK_STYLE_PROVIDER(display_provider),
                                 GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(display_provider);

    // Create grid for buttons with spacing
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_box_append(GTK_BOX(vbox), grid);

    // Button labels
    const char* button_labels[] = {
        "C", "±", "%", "÷",
        "7", "8", "9", "×",
        "4", "5", "6", "-",
        "1", "2", "3", "+",
        "0", ".", "="
    };

    // Create buttons
    for (int i = 0; i < 19; i++) {
        int row = i / 4;
        int col = i % 4;
        const char* color;

        if (i < 3) color = COLOR_FUNC_BTN;
        else if (col == 3) color = COLOR_OP_BTN;
        else if (i == 18) color = COLOR_EQ_BTN;
        else color = COLOR_NUM_BTN;

        GtkWidget *button = create_button(button_labels[i], color);

        if (i == 16) { // Zero button spans two columns
            gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 2, 1);
        } else if (i > 16) { // Dot and equals buttons
            gtk_grid_attach(GTK_GRID(grid), button, col + 1, 4, 1, 1);
        } else {
            gtk_grid_attach(GTK_GRID(grid), button, col, row, 1, 1);
        }
    }

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app = gtk_application_new("com.example.calculator",
                                            G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
