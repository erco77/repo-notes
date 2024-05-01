// vim: autoindent tabstop=8 shiftwidth=4 expandtab softtabstop=4

#ifndef EDITNOTES_H
#define EDITNOTES_H

// STL
#include <string>
#include <vector>
using namespace std;

// Fltk
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Button.H>

class EditNotesDialog : public Fl_Double_Window {
    Fl_Text_Editor *edit_;
    Fl_Text_Buffer *buff_;
    Fl_Button      *save_;
public:
    EditNotesDialog(int W=900,int H=400,const char* L=0) : Fl_Double_Window(W,H,L) {
        edit_ = new Fl_Text_Editor(0,0,W,H-35);
        buff_ = new Fl_Text_Buffer();
        edit_->buffer(buff_);
        save_ = new Fl_Button(W-110,H-30,100,25,"Save");
        end();
    }
    ~EditNotesDialog() {
        if (buff_) {
            delete buff_;
            buff_ = 0;
        }
    }
    void title(const char* val) {
        copy_label(val);
    }
    void title(const string& val) {
        copy_label(val.c_str());
    }
    void save_callback(Fl_Callback* cb, void *data=0) {
        save_->callback(cb, data);
    }
    string notes() const {
        string val = buff_->text();
        return val;
    }
    void notes(const string& val) {
        buff_->text(val.c_str());
    }
};

#endif
