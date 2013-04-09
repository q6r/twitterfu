#ifndef GTK_TWITTERFU_
#define GTK_TWITTERFU_

#include "gtkmm.h"

class GtkTwitterfu : public Gtk::Window {
    public:
        GtkTwitterfu();
        virtual ~GtkTwitterfu();
        void on_button_quit();
        void on_button_find_followers();
        void on_button_find_following();
        void on_button_start_following();
        void on_button_stop_following();
    private:
        Gtk::Button button_find_followers;
        Gtk::Button button_find_following;
        Gtk::Button button_start_following;
        Gtk::Button button_stop_following;
        Gtk::Button button_quit;

        // treeview model
        class ModelColumns : public Gtk::TreeModel::ColumnRecord {
            public:
                ModelColumns() {
                    add(m_col_text);
                    add(m_col_number);
                }
                Gtk::TreeModelColumn<Glib::ustring> m_col_text;
                Gtk::TreeModelColumn<Glib::ustring> m_col_number;
        }; // end treeviwe model

        ModelColumns columns;

        //child widgets
        Gtk::Box vbox;
        Gtk::TreeView treeview;
        Gtk::ScrolledWindow scrolledwindow;

        Glib::RefPtr< Gtk::ListStore> refTreeModel;
        Gtk::ButtonBox buttonbox;

};
#endif
