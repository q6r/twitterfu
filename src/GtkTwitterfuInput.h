#ifndef GTK_TWITTERFU_INPUT_
#define GTK_TWITTERFU_INPUT_

#include <gtkmm.h>

/*
 * This input asking window is only to be inhereted and implemented
 * by the inputWorker class.
 */
class GtkTwitterfuInput : public Gtk::Window {
    public:
        GtkTwitterfuInput(Glib::ustring text_info);
        ~GtkTwitterfuInput();
        char *getResult();
    private:
        void on_button_okay();
        char *result;
        // child widgets
        Gtk::Box vbox;
        Gtk::Button button_okay;
        Gtk::Entry entry;
    protected:
        virtual bool on_delete_event( GdkEventAny *event ) {
            delete this;
            return false;
        }
};
#endif
