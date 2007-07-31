//
// shoes/dialogs.c
// Various dialog boxes, such as for opening files and picking colors.
//
#include "shoes/app.h"
#include "shoes/ruby.h"
#include "shoes/config.h"
#ifdef SHOES_WIN32
#include "shoes/appwin32.h"
#endif

const char *dialog_title = "Shoes asks:";
const char *dialog_title_says = "Shoes says:";

#ifdef SHOES_WIN32
char *win32_dialog_label = "Ask label";
char *win32_dialog_answer = NULL;

BOOL CALLBACK
shoes_ask_win32proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:
      SetDlgItemText(hwnd, IDQUIZ, win32_dialog_label);
      if (win32_dialog_answer != NULL)
      {
        GlobalFree((HANDLE)win32_dialog_answer);
        win32_dialog_answer = NULL;
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
        {
          int len = GetWindowTextLength(GetDlgItem(hwnd, IDQUED));
          if(len > 0)
          {
            int i;
            win32_dialog_answer = (char*)GlobalAlloc(GPTR, len + 1);
            GetDlgItemText(hwnd, IDQUED, win32_dialog_answer, len + 1);
          }
        }
        case IDCANCEL:
          EndDialog(hwnd, LOWORD(wParam));
          return TRUE;
      }
      break;

    case WM_CLOSE:
      EndDialog(hwnd, 0);
      return FALSE;

    default:
      return FALSE;
  }
}
#endif

VALUE
shoes_dialog_alert(VALUE self, VALUE msg)
{
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_message_dialog_new_with_markup(
    GTK_WINDOW(global_app->kit.window), GTK_DIALOG_MODAL,
    GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "<span size='larger'>%s</span>\n\n%s",
    _(dialog_title_says), RSTRING_PTR(msg));
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_WIN32
  MessageBox(global_app->slot.window, RSTRING_PTR(msg), dialog_title_says, MB_OK);
#endif

#ifdef SHOES_QUARTZ
  OSErr err;
  SInt16 ret;
  AlertStdAlertParamRec alert;
  alert.movable = nil;
  alert.helpButton = nil;
  alert.filterProc = nil;
  alert.defaultText = kAlertDefaultOKText;
  alert.cancelText = nil;
  alert.otherText = nil;
  alert.defaultButton = kAlertStdAlertOKButton;
  alert.cancelButton = nil;
  alert.position = 0;
  err = StandardAlert(kAlertPlainAlert, "\pShoes says:", RSTRING_PTR(rb_str_to_pas(msg)), &alert, &ret);
#endif
  return Qnil;
}

VALUE
shoes_dialog_ask(VALUE self, VALUE quiz)
{
  VALUE answer = Qnil;
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_dialog_new_with_buttons(_(dialog_title),
    GTK_WINDOW(global_app->kit.window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 6);
  GtkWidget *question = gtk_label_new(RSTRING_PTR(quiz));
  gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
  GtkWidget *_answer = gtk_entry_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), question, FALSE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), _answer, FALSE, TRUE, 3);
  gtk_widget_show_all(dialog);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
  {
    const gchar *txt = gtk_entry_get_text(GTK_ENTRY(_answer));
    answer = rb_str_new2(txt);
  }
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_WIN32
  win32_dialog_label = RSTRING_PTR(quiz);
  int confirm = DialogBox(global_app->kit.instance, MAKEINTRESOURCE(ASKDLG),
    global_app->slot.window, shoes_ask_win32proc);
  if (confirm == IDOK)
  {
    if (win32_dialog_answer != NULL)
    {
      answer = rb_str_new2(win32_dialog_answer);
      GlobalFree((HANDLE)win32_dialog_answer);
      win32_dialog_answer = NULL;
    }
  }
#endif

#ifdef SHOES_QUARTZ
  Rect r;
  ControlRef edit;
  SetRect(&r, 50, 50, 450, 150);
  DialogRef dialog = NewDialog(NULL, &r, "\pShoes asks:", false, kWindowMovableModalDialogProc, (WindowPtr)-1L, true, 0, NULL);
  if (dialog) {
    SetRect(&r, 10, 10, 420, 20);
    CreateEditUnicodeTextControl(GetDialogWindow(dialog), &r, CFSTR(""), false, NULL, &edit);
    InsertDialogItem(dialog, 0, kResourceControlDialogItem, (Handle)edit, &r);
    ShowWindow(dialog);
    answer = Qtrue;
  }
#endif
  return answer;
}

VALUE
shoes_dialog_confirm(VALUE self, VALUE quiz)
{
  VALUE answer = Qfalse;
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_dialog_new_with_buttons(_(dialog_title),
    GTK_WINDOW(global_app->kit.window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 6);
  gtk_container_set_border_width(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), 6);
  GtkWidget *question = gtk_label_new(RSTRING_PTR(quiz));
  gtk_misc_set_alignment(GTK_MISC(question), 0, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), question, FALSE, FALSE, 3);
  gtk_widget_show_all(dialog);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
    answer = Qtrue;
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_WIN32
  int confirm = MessageBox(global_app->slot.window, RSTRING_PTR(quiz), dialog_title, MB_OKCANCEL);
  if (confirm == IDOK)
    answer = Qtrue;
#endif

#ifdef SHOES_QUARTZ
  OSErr err;
  SInt16 ret;
  AlertStdAlertParamRec alert;
  alert.movable = nil;
  alert.helpButton = nil;
  alert.filterProc = nil;
  alert.defaultText = kAlertDefaultOKText;
  alert.cancelText = kAlertDefaultCancelText;
  alert.otherText = nil;
  alert.defaultButton = kAlertStdAlertOKButton;
  alert.cancelButton = kAlertStdAlertCancelButton;
  alert.position = 0;
  err = StandardAlert(kAlertPlainAlert, "\pShoes asks:", RSTRING_PTR(rb_str_to_pas(quiz)), &alert, &ret);
  if (err == noErr && ret == kAlertStdAlertOKButton)
  {
    answer = Qtrue;
  }
#endif
  return answer;
}

VALUE
shoes_dialog_color(VALUE self, VALUE title)
{
  VALUE color = Qnil;
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_color_selection_dialog_new(RSTRING_PTR(title));
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_OK)
  {
    GdkColor _color;
    char colorstr[20];
    gtk_color_selection_get_current_color(
      GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel),
      &_color);
    sprintf(colorstr, "rgb(%d, %d, %d)", _color.red/256, _color.green/256, _color.blue/256);
    color = rb_str_new2(colorstr);
  }
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_WIN32
  CHOOSECOLOR cc;
  static COLORREF acrCustClr[16];
  HBRUSH hbrush;
  static DWORD rgbCurrent;

  // Initialize CHOOSECOLOR 
  ZeroMemory(&cc, sizeof(cc));
  cc.lStructSize = sizeof(cc);
  cc.hwndOwner = global_app->slot.window;
  cc.lpCustColors = (LPDWORD) acrCustClr;
  cc.rgbResult = rgbCurrent;
  cc.Flags = CC_FULLOPEN | CC_RGBINIT;
   
  if (ChooseColor(&cc)) {
    char colorstr[20];
    sprintf(colorstr, "rgb(%d, %d, %d)", GetRValue(cc.rgbResult), 
      GetGValue(cc.rgbResult), GetBValue(cc.rgbResult));
    color = rb_str_new2(colorstr);
  }
#endif

#ifdef SHOES_QUARTZ
  Point where;
  RGBColor colwh = { 0xFFFF, 0xFFFF, 0xFFFF };
  RGBColor _color;
  where.h = where.v = 0;
  if (GetColor(where, RSTRING_PTR(title), &colwh, &_color))
  {
    char colorstr[20];
    sprintf(colorstr, "rgb(%d, %d, %d)", _color.red/256, _color.green/256, _color.blue/256);
    color = rb_str_new2(colorstr);
  }
#endif
  return color;
}

VALUE
shoes_dialog_open(VALUE self)
{
  VALUE path = Qnil;
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_file_chooser_dialog_new("Open file...", GTK_WINDOW(global_app->kit.window),
    GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    path = rb_str_new2(filename);
  }
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_WIN32
  char dir[MAX_PATH], _path[MAX_PATH];
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  // GetCurrentDirectory(sizeof(dir), (LPSTR)dir);
  ofn.lStructSize     = sizeof(ofn);
  ofn.hwndOwner       = global_app->slot.window;
  ofn.hInstance       = global_app->kit.instance;
  ofn.lpstrFile       = _path;
  ofn.nMaxFile        = sizeof(_path);
  ofn.lpstrFile[0] = '\0';
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL; // (LPSTR)dir;
  ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  if (GetOpenFileName(&ofn))
  {
    path = rb_str_new2(ofn.lpstrFile);
  }
#endif

#ifdef SHOES_QUARTZ
  char _path[SHOES_BUFSIZE];
  FSRef fr;
  NavDialogRef dialog;
  NavReplyRecord reply;
  NavDialogCreationOptions opts;
  Size actualSize;
  OSErr err = noErr;
   
  NavGetDefaultDialogCreationOptions(&opts);
  err = NavCreateGetFileDialog(&opts, NULL, NULL, NULL, NULL, NULL, &dialog);
  NavDialogRun(dialog);
  err = NavDialogGetReply(dialog, &reply); 
  NavDialogDispose(dialog);
      
  if (reply.validRecord)
  {
    // Get a pointer to selected file
    err = AEGetNthPtr(&(reply.selection), 1, typeFSRef, NULL,
      NULL, &fr, sizeof(FSRef), NULL);
    FSRefMakePath(&fr, &_path, SHOES_BUFSIZE);
    path = rb_str_new2(_path);
  }
#endif
    
  return path;
}

VALUE
shoes_dialog_save(VALUE self)
{
  VALUE path = Qnil;
#ifdef SHOES_GTK
  GtkWidget *dialog = gtk_file_chooser_dialog_new("Save file...", GTK_WINDOW(global_app->kit.window),
    GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);
  gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog));
  if (result == GTK_RESPONSE_ACCEPT)
  {
    char *filename;
    filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
    path = rb_str_new2(filename);
  }
  gtk_widget_destroy(dialog);
#endif

#ifdef SHOES_QUARTZ
  char _path[SHOES_BUFSIZE];
  FSRef fr;
  NavDialogRef dialog;
  NavReplyRecord reply;
  NavDialogCreationOptions opts;
  Size actualSize;
  OSErr err = noErr;
   
  NavGetDefaultDialogCreationOptions(&opts);
  opts.modality = kWindowModalityAppModal; 
  err = NavCreatePutFileDialog(&opts, NULL, NULL, NULL, NULL, &dialog);
  NavDialogRun(dialog);
  err = NavDialogGetReply(dialog, &reply); 
  NavDialogDispose(dialog);
      
  if (reply.validRecord)
  {
    // Get a pointer to selected file
    err = AEGetNthPtr(&(reply.selection), 1, typeFSRef, NULL,
      NULL, &fr, sizeof(FSRef), NULL);
    FSRefMakePath(&fr, &_path, SHOES_BUFSIZE);
    path = rb_str_new2(_path);
    rb_str_cat2(path, "/");
    CFStringGetCString(reply.saveFileName, _path, SHOES_BUFSIZE, 0);
    rb_str_cat2(path, _path);
  }
#endif

#ifdef SHOES_WIN32
  char dir[MAX_PATH], _path[MAX_PATH];
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  // GetCurrentDirectory(sizeof(dir), (LPSTR)dir);
  ofn.lStructSize     = sizeof(ofn);
  ofn.hwndOwner       = global_app->slot.window;
  ofn.hInstance       = global_app->kit.instance;
  ofn.lpstrFile       = _path;
  ofn.nMaxFile        = sizeof(_path);
  ofn.lpstrFile[0] = '\0';
  ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.lpstrInitialDir = NULL; // (LPSTR)dir;
  ofn.Flags           = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
  if (GetSaveFileName(&ofn))
  {
    path = rb_str_new2(ofn.lpstrFile);
  }
#endif
  return path;
}
