# NAME

CopyQ - Clipboard Manager with Advanced Features

# SYNOPSIS
```sh
copyq [COMMAND]
```

# DESCRIPTION

 - **`CopyQ`**: is  advanced  clipboard  manager with editing and scripting fea‐
   tures. It monitors system clipboard and saves its content in customized
   tabs. Saved clipboard can be later copied and pasted directly into  any
   application.

# COMMANDS

 - Starts server if no command is specified.
 - **`show`** `[NAME]`:
   - Show main window and optionally open tab with given name.
 - **`hide`**: Hide main window.
 - **`toggle`**: Show or hide main window.
 - **`menu`**: Open context menu.
 - **`exit`**: Exit server.
 - **`disable,`** `enable`: Disable or enable clipboard content storing.
 - **`clipboard`** `[MIME]`: Print clipboard content.
 - **`selection`** `[MIME]`: Print X11 selection content.
 - **`paste`**: Paste clipboard to current window (may not work with some appli‐
     cations).
 - **`copy`** `TEXT`: Set clipboard text.
 - **`copy`** `MIME DATA [MIME DATA]...`: Set clipboard content.
 - **`count`**: Print amount of items in current tab.
 - **`select`** `[ROW=0]`: Copy item in the row to clipboard.
 - **`next`**: Copy next item from current tab to clipboard.
 - **`previous`**: Copy previous item from current tab to clipboard.
 - **`add`** `TEXT...`: Add text into clipboard.
 - **`insert`** `ROW TEXT`: Insert text into given row.
 - **`remove`** `[ROWS=0...]`: Remove items in given rows.
 - **`edit`** `[ROW=-1...]`:
   - Edit  items  or  edit  new one.  Value -1 is for current text in
     clipboard.
 - **`separator`** `SEPARATOR`: Set separator for items on output.
 - **`read`** `[MIME|ROW]...`: Print raw data of clipboard or item in row.
 - **`write`** `[ROW=0] MIME DATA [MIME DATA]...`: Write raw data to given row.
 - **`action`** `[ROWS=0...]`: Show action dialog.
 - **`action`** `[ROWS=0...] [PROGRAM [SEPARATOR=\n]]`:
   - Run PROGRAM on item text in the rows.  Use %1 in PROGRAM to pass
     text as argument.
 - **`popup`** `TITLE MESSAGE [TIME=8000]`:
   - Show tray popup message for TIME milliseconds.
 - **`tab`**: List available tab names.
 - **`tab`** `NAME [COMMAND]`:
   - Run command on tab with  given  name.   Tab  is  created  if  it
     doesn't exist.  Default is the first tab.
 - **`removetab`** `NAME`: Remove tab.
 - **`renametab`** `NAME NEW_NAME`: Rename tab.
 - **`exporttab`** `FILE_NAME`: Export items to file.
 - **`importtab`** `FILE_NAME`: Import items from file.
 - **`config`**: List all options.
 - **`config`** `OPTION`: Get option value.
 - **`config`** `OPTION VALUE`: Set option value.
 - **`eval,`** `-e [SCRIPT] [ARGUMENTS]...`:
   - Evaluate  ECMAScript  program.   Arguments  are accessible using
     with *"arguments[0..N]"*.
 - **`session,`** `-s, --session SESSION`:
   - Starts or connects to application instance  with  given  session
     name.
 - **`help,`** `-h, --help [COMMAND]...`:
   - Print help for COMMAND or all commands.

# NOTES

 - Use dash argument (-) to read data from standard input.
 - Use  double-dash  argument (--) to read all following arguments without
   expanding escape sequences (i.e. \n, \t and others).
 - Use ? for MIME to print available MIME types (default is *"text/plain"*).

# EXAMPLES

 - Insert some texts to the history:
     copyq add *"first item"* *"second item"* *"third item"*
 - Print content of the first three items:
     copyq read 0 1 2 copyq separator "," read 0 1 2
 - Show current clipboard content:
     copyq clipboard copyq clipboard text/html copyq clipboard  \?  \#
     lists formats in clipboard
 - Copy text to the clipboard:
     copyq copy *"Some Text"*
 - Load file content into clipboard:
     copyq  copy - < file.txt copyq copy text/html < index.html copyq
     copy image/jpeg - < image.jpg
 - Create an image items:
     copyq write image/gif - < image.gif copyq write  image/svg  -  <
     image.svg

# SEE ALSO

 - https://copyq.readthedocs.io/

# AUTHOR

 - The  maintainer of CopyQ is Lukas Holecek <hluk@email.cz>. A comprehen‐
   sive list of authors and contributors is available in the AUTHORS file.
 - This manual page was written by GengYu  Rao  (zouyoo@outlook.com),  for
   the Debian project (and may be used by others).
