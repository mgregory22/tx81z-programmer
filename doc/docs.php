<?php
# This script just opens the docs at a certain page within a frameset.
$source = 'index.html';
if (!empty($_GET)) {
  $pages = array(
      'algctrl.html'             , 'library_buttons.html'      , 'remote_control.html'
    , 'algorithm.html'           , 'library_menus.html'        , 'rpanel.html'
    , 'ampmod.html'              , 'license.html'              , 'scrollbars.html'
    , 'bundles.html'             , 'listboxes.html'            , 'snapshot.html'
    , 'buttons.html'             , 'mainwnd.html'              , 'snapshot_area.html'
    , 'comboboxes.html'          , 'mainwnd_controls.html'     , 'snapshot_buttons.html'
    , 'controls.html'            , 'mainwnd_menu.html'         , 'snapshot_files.html'
    , 'dialogs.html'             , 'menus.html'                , 'snapshot_menu.html'
    , 'edit_buffers.html'        , 'microtunings_editors.html' , 'synchronization.html'
    , 'editable_items.html'      , 'midi_keyboard.html'        , 'system_editor.html'
    , 'effects_editor.html'      , 'misc.html'                 , 'template.html'
    , 'eg.html'                  , 'mysteries.html'
    , 'egbias.html'              , 'note.html'                 , 'tx81z_notes.html'
    , 'frequency.html'           , 've_eggraphs.html'
    , 'glossary.html'            , 'output.html'               , 've_lcdctrls.html'
    , 'index.html'               , 'pctable_editor.html'       , 've_menubtns.html'
    , 'invokation.html'          , 'pe_controls.html'          , 've_menus.html'
    , 'item_types.html'          , 'pe_menus.html'             , 've_multiedit.html'
    , 'keyboard_navigation.html' , 'performance_editor.html'   , 've_scrolling.html'
    , 'lcdctrl.html'             , 'pitchbias.html'            , 've_simplefreq.html'
    , 'lfo.html'                 , 'pitchmod.html'             , 'voice_editor.html'
    , 'libraries.html'           , 'portamento.html'           , 'vol.html'
    , 'library_area.html'        , 'quickstart.html'           , 'windows.html'
  );
  foreach ($_GET as $param => $dummy) {
      $param = str_replace('_html', '.html', $param);
      $param = str_replace('res/', '', $param);
      if (in_array($param, $pages)) {
          $source = $param;
      }
  }
}
?>
<html>

<head>
<title>TX81Z Programmer Manual</title>
<link rel="stylesheet" href="styles.css" type="text/css">
</head>

<frameset cols="300,*">
  <frame src="toc.html" name="toc">
  <frame src="<?php echo $source ?>" name="content">
</frameset>

</html>

