# NAME

mpv - a media player

# SYNOPSIS
```sh
mpv [options] [file|URL|PLAYLIST|-]
mpv [options] files
```

# DESCRIPTION

 - mpv is a media player based on MPlayer and mplayer2. It supports a wide
   variety  of  video  file  formats, audio and video codecs, and subtitle
   types. Special input URL types are available to read input from a vari‐
   ety of sources other than disk files. Depending on platform, a  variety
   of different video and audio output methods are supported.

 - **`Usage`**: examples  to  get you started quickly can be found at the end of
   this man page.

# INTERACTIVE CONTROL
 - See also `--input-test` for interactive binding details by key,  and  the
   stats  built-in script for key bindings list (including print to termi‐
   nal).
**Keyboard Control**

 - **`LEFT`** `and RIGHT`:
   - Seek backward/forward 5 seconds. Shift+arrow does a 1 second ex‐
     act seek (see `--hr-seek`).
 - **`UP`** `and DOWN`:
   - Seek forward/backward 1 minute. Shift+arrow does a 5 second  ex‐
     act seek (see `--hr-seek`).
 - **`Ctrl+LEFT`** `and Ctrl+RIGHT`:
   - Seek to the previous/next subtitle. Subject to some restrictions
     and might not always work; see sub-seek command.
 - **`Ctrl+Shift+Left`** `and Ctrl+Shift+Right`:
   - Adjust  subtitle  delay so that the next or previous subtitle is
     displayed now. This is especially useful to  sync  subtitles  to
     audio.
 - **`[`** `and ]`: Decrease/increase current playback speed by 10%.
 - **`{`** `and }`: Halve/double current playback speed.
 - **`BACKSPACE`**: Reset playback speed to normal.
 - **`Shift+BACKSPACE`**:
   - Undo  the  last  seek. This works only if the playlist entry was
     not changed.  Hitting it a second time will go back to the orig‐
     inal position.  See revert-seek command for details.
 - **`Shift+Ctrl+BACKSPACE`**:
   - Mark the current position. This will then be used by Shift+BACK‐
     SPACE as revert position (once you seek back, the marker will be
     reset). You can use this to seek around in the file and then re‐
     turn to the exact position where you left off.
 - **`<`** `and >`: Go backward/forward in the playlist.
 - **`ENTER`**: Go forward in the playlist.
 - **`p`** `/ SPACE`: Pause (pressing again unpauses).
 - **`.`**: Step forward. Pressing once will pause, every consecutive  press
     will play one frame and then go into pause mode again.
 - **`,`**: Step backward. Pressing once will pause, every consecutive press
     will  play  one  frame  in  reverse  and then go into pause mode
     again.
 - **`q`**: Stop playing and quit.
 - **`Q`**: Like q, but store the current  playback  position.  Playing  the
     same file later will resume at the old playback position if pos‐
     sible. See RESUMING PLAYBACK.
 - **`/`** `and *`: Decrease/increase volume.
 - **`9`** `and 0`: Decrease/increase volume.
 - **`Ctrl`** `s`: Take  a screenshot, as the window shows it (with subtitles, OSD,
     and scaled video).
 - **`PGUP`** `and PGDWN`:
   - Seek to the beginning of  the  previous/next  chapter.  In  most
     cases,  *"previous"* will actually go to the beginning of the cur‐
     rent chapter; see `--chapter-seek-threshold`.
 - **`Ctrl`** `h`: Toggle hardware video decoding on/off.
 - **`Alt+LEFT,`** `Alt+RIGHT, Alt+UP, Alt+DOWN`:
   - Move the video rectangle (panning).
 - **`Alt`** `+ and Alt -`:
   - Combining Alt with the + or - keys changes video zoom.
 - **`F8`**: Show the playlist and the current position in it (useful only if
     a UI window is used, broken on the terminal).
 - **`F9`**: Show the list of audio and subtitle streams (useful only if a UI
     window  is used, broken on the terminal).
 - **`i`** `and I`:
   - Show/toggle an overlay displaying statistics about the currently
     and so on. See STATS for more information.
 - **```**: Show the console. (ESC closes it again. See CONSOLE.)

# USAGE

 - The fixed-length quoting syntax  is  intended  for  use  with  external
   scripts and programs.
 - It is started with % and has the following format:
 - %n%string_of_length_n
 - Examples
   - mpv '`--vf=foo:option1=`%11%quoted text' test.avi
   - Or in a script:
   - mpv `--vf=foo:option1=`%`expr length "`$NAME`"`%"`$NAME`" test.avi
 - **`Note:`** `where applicable with JSON-IPC, %n% is the length in UTF-8 bytes,`:
   - after decoding the JSON data.
**Profiles**

 - You  can  list profiles with `--profile=help`, and show the contents of a
   profile with `--show-profile=<name>` (replace  <name>  with  the  profile
   name).  You  can  apply profiles on start with the `--profile=<name>` op‐
   tion, or at runtime with the apply-profile <name> command.
 - Example mpv config file with profiles
```sh
# normal top-level option
fullscreen=yes
```
```sh
# a profile that can be enabled with --profile=big-cache
[big-cache]
cache=yes
demuxer-max-bytes=123400KiB
demuxer-readahead-secs=20
```
```sh
[slow]
profile-desc="some profile name"
# reference a builtin profile
profile=gpu-hq
```
```sh
[fast]
vo=vdpau
```
```sh
# using a profile again extends it
[slow]
framedrop=no
# you can also include other profiles
profile=big-cache
```

**Runtime profiles**

 - Profiles can be set at runtime with apply-profile command.  Since  this
   operation is *"destructive"* (every item in a profile is simply set as an
   option, overwriting the previous value), you can't just enable and dis‐
   able profiles again.
 - As  a  partial  remedy, there is a way to make profiles save old option
   values before overwriting  them  with  the  profile  values,  and  then
   restoring  the  old  values  at a later point using apply-profile <pro‐
   file-name> restore.
 - This can be enabled with the profile-restore option, which takes one of
   the following options:

 - **`default`**: Does nothing, and nothing can be restored (default).
 - **`copy`**: When applying a profile, copy the old values of  all  profile
     options  to  a  backup  before setting them from the profile.
     These options are reset to their old values using the  backup
     when restoring.
 - **`copy-equal`**:
   - Similar  to  copy,  but  restore an option only if it has the
     same value as the value effectively set by the profile.  This
     tries  to deal with the situation when the user does not want
     the option to be reset after interactively changing it.
 - Example
```sh
[something]
profile-restore=copy-equal
vf-add=rotate=PI/2  # rotate by 90 degrees
```
 - Then running these commands will result in behavior as commented:
```sh
set vf vflip
apply-profile something
vf add hflip
apply-profile something
# vf == vflip,rotate=PI/2,hflip,rotate=PI/2
apply-profile something restore
# vf == vflip
```

**Conditional auto profiles**

 - Recursive  profiles  can  be  used.  But it is discouraged to reference
   other conditional profiles in a conditional  profile,  since  this  can
   lead to tricky and unintuitive behavior.
 - Example
   - Make only HD video look funny:
```sh
[something]
profile-desc=HD video sucks
profile-cond=width >= 1280
hue=-50
```
 - Make  only  videos  containing *"youtube"* or *"youtu.be"* in their path
   brighter:
```sh
[youtube]
profile-cond=path:find('youtu%.?be')
gamma=20
```
 - If you want the profile to be reverted  if  the  condition  goes  to
   false again, you can set profile-restore:
```sh
[something]
profile-desc=Mess up video when entering fullscreen
profile-cond=fullscreen
profile-restore=copy
vf-add=rotate=PI/2  # rotate by 90 degrees
```
 - This appends the rotate filter to the video filter chain when enter‐
   ing fullscreen. When leaving fullscreen, the vf option is set to the
   value  it  had before entering fullscreen. Note that this would also
   remove any other filters that were added during fullscreen  mode  by
   the user. Avoiding this is trickier, and could for example be solved
   by adding a second profile with an inverse condition and operation:
```sh
[something]
profile-cond=fullscreen
vf-add=@rot:rotate=PI/2
```
```sh
[something-inv]
profile-cond=not fullscreen
vf-remove=@rot
```
 - WARNING:
   Every  time an involved property changes, the condition is evaluated
   again.  If your condition uses p.playback_time for example, the con‐
   dition is re-evaluated approximately on every video frame.  This  is
   probably slow.
 - This  feature is managed by an internal Lua script. Conditions are exe‐
   cuted as Lua code within this script. Its environment contains at least
   the following things:

 - **`(function`** `environment table)`:
   - Every Lua function has an environment table. This  is  used  for
     identifier  access.  There  is no named Lua symbol for it; it is
     implicit.
 - **`p`**: A  *"magic"*  table  similar  to the environment table. Unlike the
     latter, this does not prefer accessing variables defined in _G -
     it always accesses properties.
 - **`get(name`** `[, def])`:
   - Read a property and return its value. If the property  value  is
     nil (e.g.  if the property does not exist), def is returned.
 - In addition, the same environment as in  a  blank  mpv  Lua  script  is
   present. For example, math is defined and gives access to the Lua stan‐
   dard math library.
 - WARNING:
   This  feature is subject to change indefinitely. You might be forced
   to adjust your profiles on mpv updates.
**Legacy auto profiles**

 - Some profiles are loaded automatically using a  legacy  mechanism.  The
   following example demonstrates this:
 - Auto profile loading
```sh
[extension.mkv]
profile-desc="profile for .mkv files"
vf=vflip
```
 - The profile name follows the schema type.name, where type can be proto‐
   col  for  the  input/output protocol in use (see `--list-protocols`), and
   extension for the extension of the path of the  currently  played  file
   (not the file format).
 - This  feature  is  very limited, and is considered soft-deprecated. Use
   conditional auto profiles.

# USING MPV FROM OTHER PROGRAMS OR SCRIPTS

 - There are three choices for using mpv from other programs or scripts:

   - 1. Calling it as UNIX process. If you do this, do not parse terminal
     output.  The terminal output is  intended  for  humans,  and  may
     change any time. In addition, terminal behavior itself may change
     any time. Compatibility cannot be guaranteed.
   - 2. Using  libmpv.  This is generally recommended when mpv is used as
     playback backend for a completely different application. The pro‐
     vided C API is very close to CLI  mechanisms  and  the  scripting
     API.
   - Note  that  even  though libmpv has different defaults, it can be
     configured to work exactly like the CLI  player  (except  command
     line parsing is unavailable).
   - See EMBEDDING INTO OTHER PROGRAMS (LIBMPV).
   - 3. As a user script (LUA SCRIPTING, JAVASCRIPT, C PLUGINS). This is
     recommended when the goal is to *"enhance"* the CLI player. Scripts
     get access to the entire client API of mpv.
   - This is the standard way to create third-party extensions for the
     player.
 - All these access the client API, which is the sum of the various mecha‐

# TERMINAL STATUS LINE

 - During playback, mpv shows the playback  status  on  the  terminal.  It
   looks like something like this:
   AV: 00:03:12 / 00:24:25 (13%) A-V: -0.000
 - The status line can be overridden with the `--term-status-msg` option.
 - The  following is a list of things that can show up in the status line.
   Input properties, that can be used to get the  same  information  manu‐
   ally, are also listed.
 - AV: or V: (video only) or A: (audio only)
 - The current time position in HH:MM:SS format (playback-time property)
 - The total file duration (absent if unknown) (duration property)
 - Playback  speed,  e.g. x2.0. Only visible if the speed is not normal.
   This is the user-requested speed, and not the actual speed   (usually
   they  should  be the same, unless playback is too slow). (speed prop‐
   erty.)

# LOW LATENCY PLAYBACK

 - mpv  is  optimized for normal video playback, meaning it actually tries
   to buffer as much data as it seems to make sense.  This  will  increase
   latency.  Reducing  latency  is possible only by specifically disabling
   features which increase latency.
 - Additional options that can be tried:
 - `--opengl-glfinish=yes`, can reduce buffering in the graphics driver
 - `--opengl-swapinterval=0`, same
 - `--vo=xv`, same
 - without audio `--framedrop=no` `--speed=1`.01 may help for  live  sources
   (results can be mixed)

# PROTOCOLS

 - **`http://...,`** `https://, ...`:
   - Many network protocols are supported, but the protocol  prefix  must
   always be specified. mpv will never attempt to guess whether a file‐
   name  is actually a network address. A protocol prefix is always re‐
   quired.
 **`-`**: Play data from stdin.
 - **`smb://PATH`**: Play a path from  Samba share. (Requires FFmpeg support.)
 - **`bd://[title][/device]`** `--bluray-device=PATH`:
   - Play  a  Blu-ray  disc. Since libbluray 1.0.1, you can read from ISO
   files by passing them to `--bluray-device`.
 - title can be: longest  or  first  (selects  the  default  playlist);
   mpls/<number>  (selects  <number>.mpls  playlist);  <number> (select
   playlist with the same index). mpv will list the available playlists
   on loading.
 - bluray:// is an alias.
 - **`dvd://[title][/device]`** `--dvd-device=PATH`:
   - Play a DVD. DVD menus are not supported. If no title is  given,  the
   longest title is auto-selected. Without `--dvd-device`, it will proba‐
   bly  try  to  open  an actual optical drive, if available and imple‐
   mented for the OS.
 - dvdnav:// is an old alias for  dvd://  and  does  exactly  the  same
   thing.
 - **`av://type:options`**:
   - This  is intended for using libavdevice inputs. type is the libavde‐
   vice demuxer name, and options is the  (pseudo-)filename  passed  to
   the demuxer.
   - Example
   - mpv av://v4l2:/dev/video0 `--profile=low-latency` `--untimed`
   - This plays video from the first v4l input with nearly the lowest
     latency  possible. It's a good replacement for the removed tv://
     input.  Using `--untimed` is a hack to output a captured frame im‐
     mediately, instead of respecting the input framerate. (There may
     be better ways to handle this in the future.)
 - avdevice:// is an alias.
 - **`file://PATH`**:
   - A local path as URL. Might be useful in some special use-cases. Note
   that PATH itself should start with a third / to make the path an ab‐
   solute path.
 - **`slice://start[-end]@URL`**: Read a slice of a stream.
 - start and end represent a byte range and accept suffixes such as KiB
   and MiB. end is optional.
 - if end starts with +, it is considered as offset from start.
 - Only works with seekable streams.
 - Examples:
   - mpv slice://1g-2g@cap.ts
   - This starts reading from cap.ts after seeking 1 GiB, then
     reads until reaching 2 GiB or end of file.
   - mpv slice://1g-+2g@cap.ts
   - This starts reading from cap.ts after seeking 1 GiB, then
     reads until reaching 3 GiB or end of file.
   - mpv slice://100m@appending://cap.ts
   - This starts reading from cap.ts after seeking 100MiB, then
     reads until end of file.
 - **`null://`**:
   - Simulate  an  empty file. If opened for writing, it will discard all
   data.  The null demuxer will specifically pass autoprobing  if  this
   protocol  is  used  (while  it's not automatically invoked for empty
   files).
 - **`memory://data`**: Use the data part as source data.
 - **`hex://data`**: Like memory://, but the string is interpreted as hexdump.

# PSEUDO GUI MODE

 - mpv has no official GUI, other than the  OSC  (ON  SCREEN  CONTROLLER),
   which  is not a full GUI and is not meant to be. However, to compensate
   for the lack of expected GUI behavior, mpv will  in  some  cases  start
   with some settings changed to behave slightly more like a GUI mode.
 - Currently this happens only in the following cases:
 - if  started  using  the  mpv.desktop file on Linux (e.g. started from
   menus or file associations provided by desktop environments)
 - if started from explorer.exe  on  Windows  (technically,  if  it  was
   started  on  Windows,  and all of the stdout/stderr/stdin handles are
   unset)
 - started out of the bundle on macOS
 - if you manually use `--player-operation-mode=pseudo-gui` on the command
   line
 - The profile is currently defined as follows:
```sh
[builtin-pseudo-gui]
terminal=no
force-window=yes
idle=once
screenshot-directory=~~desktop/
```
 - The  pseudo-gui  profile  exists  for compatibility. The options in the
   file=pseudo-gui works like in older mpv releases:
```sh
[pseudo-gui]
player-operation-mode=pseudo-gui
```

# OPTIONS

**Track Selection**

 **`--alang`**=`<languagecode[,languagecode,...]>`:
   - The  special  value *"auto"* can be included anywhere in the list,
     and is equivalent to the user's OS-level list of preferred  lan‐
     guages.
   - This is a string list option. See List Options for details.
   - Examples
   - mpv  dvd://1  `--alang=hu`,en  chooses the Hungarian language
       track on a DVD and falls back on English  if  Hungarian  is
       not available.
     - mpv  `--alang=jpn`  example.mkv  plays  a  Matroska file with
       Japanese audio.
 **`--aid`**=`<ID|auto|no>`:
   - Select audio track. auto selects the default, no disables audio.
     See  also `--alang`. mpv normally prints available audio tracks on
     the terminal when starting playback of a file.
   - `--audio` is an alias for `--aid`.
   - `--aid=no` or `--audio=no` or `--no-audio`  disables  audio  playback.
     (The latter variant does not work with the client API.)
   - NOTE:
     The  track  selection  options  (`--aid` but also `--sid` and the
     others) sometimes expose behavior that  may  appear  strange.
     Also,  the  behavior tends to change around with each mpv re‐
     lease.
 **`--sid`**=`<ID|auto|no>`:
   - Display  the subtitle stream specified by <ID>. auto selects the
     default, no disables subtitles.
**Playback Control**

 **`--start`**=`<relative time>`: Seek to given time position.
   - The following alternative time specifications are recognized:
   - pp% seeks to percent position pp (0-100).
   - \#c seeks to chapter number c. (Chapters start from 1.)
   - none resets any previously set option (useful for libmpv).
   - If  `--rebase-start-time=no` is given, then prefixing times with +
     to seek to negative timestamps (useful for debugging at most).
   - Examples
   **`--start`**=`+56, --start=00:56`: Seeks to the start time + 56 seconds.
   **`--start`**=`-56, --start=-00:56`: Seeks to the end time - 56 seconds.
 **`--end`**=`<relative time>`:
   - Stop  at given time. Use `--length` if the time should be relative
     to `--start`. See `--start` for valid option values and examples.
 **`--framedrop`**=`<mode>`:
   - Skip displaying some frames to maintain A/V sync  on  slow  sys‐
     tems, or playing high framerate video on video outputs that have
     an upper framerate limit.
   - The  argument  selects  the  drop methods, and can be one of the
     following:
   - **`<no>`**: Disable any frame dropping. Not recommended, for  testing
       only.
   - **`<vo>`**: Drop  late  frames  on video output (default). This still
       line as Dropped: field.
     - In audio sync. mode, this drops frames that are  outdated
       stops  if the effective framerate is below 10 FPS.
   - **`<decoder+vo>`**:
     - Enable  both modes. Not recommended. Better than just de‐
       coder mode.
   - NOTE:
     `--vo=vdpau` has its own code for the vo framedrop mode. Slight
     differences to other VOs are possible.
 **`--audio-channels`**=`<auto-safe|auto|layouts>`:
   - Control which audio  channels  are  output  (e.g.  surround  vs.
     stereo). There are the following possibilities:
   -
   **`--audio-channels`**=`auto-safe`:
     - Use  the system's preferred channel layout. If there is
       none (such as when accessing a hardware device  instead
       of  the system mixer), force stereo. Some audio outputs
       might simply accept any layout  and  do  downmixing  on
       their own.
     - This is the default.
   -
   **`--audio-channels`**=`auto`:
     - Send  the  audio device whatever it accepts, preferring
       the audio's original channel layout. Can  cause  issues
       with HDMI (see the warning below).
 **`--gapless-audio`**=`<no|yes|weak>`:
   - Try  to  play consecutive audio files with no silence or disrup‐
     tion at the point of file change. Default: weak.
   - **`no`**: Disable gapless audio.
   - **`yes`**: The audio device is opened using  parameters  chosen  for
       shared output format will be.
   - **`weak`**: Normally, the audio device is kept open (using the format
       audio, trying to use gapless is also explicitly given up.
   - NOTE:
     This  feature is implemented in a simple manner and relies on
     can start.
 - **`video-params`**:
   - Video  parameters, as output by the decoder (with overrides like
     aspect etc. applied). This has a number of sub-properties:
   - **`video-params/pixelformat`**:
     - The pixel format as string. This uses the same  names  as
       used in other places of mpv.
   - When querying the property with the client  API  using  MPV_FOR‐
     MAT_NODE, or with Lua mp.get_property_native, this will return a
     mpv_node with the following contents:
   - **`MPV_FORMAT_NODE_MAP`**: "pixelformat"       MPV_FORMAT_STRING
       *"hw-pixelformat"*    MPV_FORMAT_STRING
       *"w"*                 MPV_FORMAT_INT64
       *"h"*                 MPV_FORMAT_INT64
       *"dw"*                MPV_FORMAT_INT64
       *"dh"*                MPV_FORMAT_INT64
       *"aspect"*            MPV_FORMAT_DOUBLE
       *"par"*               MPV_FORMAT_DOUBLE
       *"colormatrix"*       MPV_FORMAT_STRING
       *"colorlevels"*       MPV_FORMAT_STRING
       *"primaries"*         MPV_FORMAT_STRING
       *"gamma"*             MPV_FORMAT_STRING
       *"sig-peak"*          MPV_FORMAT_DOUBLE
       *"light"*             MPV_FORMAT_STRING
       *"chroma-location"*   MPV_FORMAT_STRING
       *"rotate"*            MPV_FORMAT_INT64
       *"stereo-in"*         MPV_FORMAT_STRING
       *"average-bpp"*       MPV_FORMAT_INT64
       *"alpha"*             MPV_FORMAT_STRING
 - **`dwidth,`** `dheight`:
   - Video display size. This is the video size after filters and as‐

# CONSOLE

 - The  console  is  a REPL for mpv input commands. It is displayed on the
   video window. It also shows log messages. It can be  disabled  entirely
   using the `--load-osd-console=no` option.
**Keybindings**

 - **```**: Show the console.
 - **`ESC`**: Hide the console.
 - **`ENTER,`**: Ctrl+J and Ctrl+M
     Run the typed command.
 - **`Shift+ENTER`**: Type a literal newline character.
 - **`LEFT`** `and Ctrl+B`: Move the cursor to the previous character.
 - **`RIGHT`** `and Ctrl+F`: Move the cursor to the next character.
 - **`Ctrl+LEFT`** `and Alt+B`:
   - Move  the cursor to the beginning of the current word, or if be‐
     tween words, to the beginning of the previous word.
 - **`Ctrl+RIGHT`** `and Alt+F`:
   - Move the cursor to the end of the current word,  or  if  between
     words, to the end of the next word.
 - **`HOME`** `and Ctrl+A`: Move the cursor to the start of the current line.
 - **`END`** `and Ctrl+E`: Move the cursor to the end of the current line.
 - **`BACKSPACE`** `and Ctrl+H`: Delete the previous character.
**Commands**

 - **`script-message-to`** `console type <text> [<cursor_pos>]`:
   - Show the console and pre-fill it with the provided text, option‐
     ally specifying the initial cursor position as a positive  inte‐
     ger starting from 1.
   - Example for input.conf
     - %  script-message-to console type *"seek  absolute-per‐
                        cent"* 6
**Known issues**

 - Pasting text is slow on Windows
 - Non-ASCII keyboard input has restrictions
 - The cursor keys move between Unicode code-points, not grapheme  clus‐
   ters

**Configurable Options**

 - **`scale`**: Default: 1
   - All drawing is scaled by this value, including the text  borders
     and the cursor.
   - If  the VO backend in use has HiDPI scale reporting implemented,
     the option value is scaled with the reported HiDPI scale.
 - **`font`**: Default: unset (picks a hardcoded  font  depending  on  detected
     platform)
   - Set  the  font  used for the REPL and the console. This probably
     doesn't have to be a monospaced font.
 - **`mp.add_periodic_timer(seconds,`** `fn)`:
   - Call  the given function periodically. This is like mp.add_time‐
     out, but the timer is re-added after the function fn is run.
   - Returns a timer object. The timer object provides the following
     methods:
     - **`stop()`**: Disable the timer. Does nothing if  the  timer  is
         already  disabled.  This will remember the current
         elapsed time when stopping, so that  resume()  es‐
         sentially unpauses the timer.
     - **`resume()`**: Restart the timer. If the timer was disabled  with
         timeout.
     - **`oneshot`** `(RW)`: Whether  the  timer  is  periodic (false) or fires
         tion fn is run).
   - Note  that  these are methods, and you have to call them using :
     instead           of           .            (Refer            to
     https://www.lua.org/manual/5.2/manual.html\#3.4.9 .)
   - Example:
```sh
seconds = 0
timer = mp.add_periodic_timer(1, function()
    print("called every second")
    # stop it after 10 seconds
    seconds = seconds + 1
    if seconds >= 10 then
        timer:kill()
    end
end)
```

 - **`mp.get_opt(key)`**:
   - Return  a  setting from the `--script-opts` option. It's up to the
     collisions.
**mp.msg functions**

 - This  module  allows  outputting  messages  to the terminal, and can be
   loaded with require 'mp.msg'.

 - **`msg.log(level,`** `...)`:
   - The level parameter is the message priority. It's a  string  and
     Normally, all messages are visible, except v, debug and trace.
 - **`msg.fatal(...),`** `msg.error(...), msg.warn(...), msg.info(...), msg.ver‐`:
   - bose(...), msg.debug(...), msg.trace(...)
     All  of  these are shortcuts and equivalent to the corresponding
     msg.log(level, ...) call.
**mp.options functions**

 - mpv comes with a built-in module to manage  options  from  config-files
   mand-line (in that order).

 - **`options.read_options(table`** `[, identifier [, on_update]])`:
   - A  table with key-value pairs. The type of the default values is
     command-line back. Do not use nil as a default value!
 - **`utils.file_info(path)`**:
   - Stats the given path for information and returns  a  table  with
     the following entries:
   - **`mode`**: protection  bits  (on  Windows, always 755 (octal) for
       directories and 644 (octal) for files)
   - **`size`**: size in bytes
   - **`atime`**: time of last access
   - **`mtime`**: time of last modification
   - **`ctime`**: time of last metadata change
   - **`is_file`**: Whether path is a regular file (boolean)
   - **`is_dir`**: Whether path is a directory (boolean)
   - mode and size are integers.  Timestamps (atime, mtime and ctime)
     are integer seconds since  the  Unix  epoch  (Unix  time).   The
     booleans  is_file and is_dir are provided as a convenience; they
     can be and are derived from mode.
   - On error (e.g. path does not exist), nil, error is returned.
**Asynchronous commands**

 - Command  can be run asynchronously. This behaves exactly as with normal
   command execution, except that execution is not  blocking.  Other  com‐
   mands  can  be sent while it's executing, and command completion can be
   arbitrarily reordered.
 - The async field controls this. If present, it must  be  a  boolean.  If
   missing, false is assumed.
 - For example, this initiates an asynchronous command:
```sh
{ "command": ["screenshot"], "request_id": 123, "async": true }
```
 - And this is the completion:
```sh
{"request_id":123,"error":"success","data":null}
```
 - By  design,  you  will  not  get  a  confirmation  that the command was
   started. If a command is long running, sending  the  message  will  not
   lead to any reply until much later when the command finishes.

**Commands**

 - In addition to the commands described in List of Input Commands, a  few
   extra commands can also be used as part of the protocol:

 - **`set_property`**:
   - Set the given property to the given value.  See  Properties  for
     more information about properties.
   - Example:
```sh
{ "command": ["set_property", "pause", true] }
{ "error": "success" }
```

 - **`set_property_string`**:
   - Alias  for  set_property. Both commands accept native values and
     strings.
 - **`observe_property`**:
   - Watch a property for changes. If the given property is  changed,
     then an event of type property-change will be generated
   - Example:
```sh
{ "command": ["observe_property", 1, "volume"] }
{ "error": "success" }
{ "event": "property-change", "id": 1, "data": 52.0, "name": "volume" }
```
   - WARNING:
     If  the connection is closed, the IPC client is destroyed in‐
     tion open to make it work.
 - **`observe_property_string`**:
   - Like observe_property, but the resulting data will always  be  a
     string.
   - Example:
```sh
{ "command": ["observe_property_string", 1, "volume"] }
{ "error": "success" }
{ "event": "property-change", "id": 1, "data": "52.000000", "name": "volume" }
```

**JSON extensions**

 - The following non-standard extensions are supported:
 - a list or object item can have a trailing ","
 - object syntax accepts "=" in addition of ":"
 - object  keys  can  be  unquoted, if they start with a character in
     *"A-Za-z_"* and contain only characters in *"A-Za-z0-9_"*
   - byte escapes with *"xAB"* are allowed (with AB being a 2  digit  hex
     number)
 - Example:
```sh
{ objkey = "value\x0A" }
```
 - Is equivalent to:
```sh
{ "objkey": "value\n" }
```
 - libdvdcss:

   - **`DVDCSS_CACHE`**:
     - Specify a directory in which to store title  key  values.
       special value *"off"* disables caching.
   - **`DVDCSS_METHOD`**:
     - Sets  the authentication and decryption method that libd‐
       vdcss will use to read scrambled discs. Can be one of ti‐
       tle, key or disc.
     - **`key`**: is the default method. libdvdcss will use a set of
         calculated player keys to try to get the disc key.
         This can fail if the drive does not recognize  any
         of the player keys.
     - **`disc`**: is  a fallback method when key has failed. Instead
         of using player keys,  libdvdcss  will  crack  the
         disc  key  using  a  brute  force  algorithm. This
         process is CPU intensive and  requires  64  MB  of
         memory to store temporary data.
   - **`DVDCSS_VERBOSE`**: Sets the libdvdcss verbosity level.
     - **`0`**: Outputs no messages at all.
     - **`1`**: Outputs error messages to stderr.
     - **`2`**: Outputs  error  messages  and  debug  messages  to
         stderr.
   - **`DVDREAD_NOKEYS`**:
     - Skip retrieving all keys on startup. Currently disabled.
   - **`HOME`**: FIXME: Document this.

# EXIT CODES

 - Normally  mpv  returns 0 as exit code after finishing playback success‐
   fully.  If errors happen, the following exit codes can be returned:

 - **`1`**: Error initializing mpv. This is also returned if unknown  op‐
     tions are passed to mpv.
 - **`2`**: The  file  passed to mpv couldn't be played. This is somewhat
     fuzzy: currently, playback of a file is considered to be suc‐
     cessful if initialization  was  mostly  successful,  even  if
     playback fails immediately after initialization.
 - **`3`**: There  were  some  files that could be played, and some files
     which couldn't (using the definition of success from above).
 - **`4`**: Quit due to a signal, Ctrl+c in a VO window (by default),  or
     from the default quit key bindings in encoding mode.
 - Note that quitting the player manually will always lead to exit code 0,
   overriding  the  exit  code  that would be returned normally. Also, the
   quit input command can take an exit code: in this case, that exit  code
   is returned.

# FILES

 - Note  that this section assumes Linux/BSD. On other platforms the paths
   may be different.  For Windows-specifics, see FILES ON WINDOWS section.
 - **`/usr/local/etc/mpv/mpv.conf`**:
   - mpv system-wide settings (depends on `--prefix` passed to  config‐
     ure  - mpv in default configuration will use /usr/local/etc/mpv/
     as config directory, while most Linux distributions will set  it
     to /etc/mpv/).
 - **`~/.cache/mpv`**:
   - The  standard  cache  directory.  Certain options within mpv may
     cause it to write cache files to disk. This can be overridden by
     environment variables, in ascending order:
   - **`1`**: If `$XDG_CACHE_HOME` is set, then the derived cache  direc‐
       tory will be `$XDG_CACHE_HOME`/mpv.
   - **`2`**: If  `$MPV_HOME`  is  set,  then the derived cache directory
       will be `$MPV_HOME`.
   - If the directory does not exist, mpv will try to create it auto‐
     matically.
 - **`~/.config/mpv/mpv.conf`**:
   - mpv user settings (see CONFIGURATION FILES section)
 - **`~/.config/mpv/input.conf`**: key bindings (see INPUT.CONF section)
 - **`~/.config/mpv/fonts.conf`**:
   - Fontconfig fonts.conf that is customized for mpv. You should in‐
     clude system fonts.conf in this file or mpv would not know about
     fonts that you already have in the system.
   - Only available when libass is built with fontconfig.
 - **`~/.config/mpv/subfont.ttf`**: fallback subtitle font
 - **`~/.config/mpv/fonts/`**:
   - Default  location  for  `--sub-fonts-dir`  (see   Subtitles)   and
     `--osd-fonts-dir` (see OSD).
 - **`~/.config/mpv/scripts/`**:
   - All files in this directory are loaded as if they were passed to
     the `--script` option. They are loaded in alphabetical order.
   - The `--load-scripts=no` option disables loading these files.
   - See Script location for details.

# COPYRIGHT

 - GPLv2+
