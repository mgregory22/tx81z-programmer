HHC="C:\Program Files (x86)\HTML Help Workshop\hhc.exe"
DEPS=res/algctrl.html res/algorithm.html res/ampmod.html res/bundles.html res/buttons.html \
	res/comboboxes.html res/controls.html res/dialogs.html res/edit_buffers.html \
	res/editable_items.html res/effects_editor.html res/eg.html res/egbias.html \
	res/frequency.html res/glossary.html res/index.html res/invokation.html \
	res/item_types.html res/keyboard_navigation.html res/lcdctrl.html res/lfo.html \
	res/libraries.html res/library_area.html res/library_buttons.html \
	res/library_menus.html res/license.html res/listboxes.html res/mainwnd.html \
	res/mainwnd_controls.html res/mainwnd_menu.html res/menus.html \
	res/microtunings_editors.html res/midi_keyboard.html res/misc.html res/mysteries.html \
	res/note.html res/output.html res/pctable_editor.html \
	res/pe_controls.html res/pe_menus.html res/performance_editor.html res/pitchbias.html \
	res/pitchmod.html res/portamento.html res/quickstart.html res/remote_control.html \
	res/rpanel.html res/scrollbars.html res/snapshot.html res/snapshot_area.html \
	res/snapshot_buttons.html res/snapshot_files.html res/snapshot_menu.html \
	res/synchronization.html res/system_editor.html res/template.html \
	res/tx81z_notes.html res/ve_eggraphs.html res/ve_lcdctrls.html res/ve_menubtns.html \
	res/ve_menus.html res/ve_multiedit.html res/ve_scrolling.html res/ve_simplefreq.html \
	res/voice_editor.html res/vol.html res/windows.html res/algctrl.gif res/algorithm.gif \
	res/ampmod.gif res/bndldlg.gif res/checkbox.gif res/combobox.gif res/datadlg.gif \
	res/dataflow.gif res/dialog.gif res/diff.gif res/editkeys.gif res/eg.gif res/egbias.gif \
	res/EGgraphs.gif res/excel.gif res/exportdlg.gif res/frequency.gif res/fxdlg.gif \
	res/importdlg.gif res/keynav.gif res/lcdctrl.gif res/lfo.gif res/library_area.gif \
	res/library_buttons.gif res/library_file_menu.gif res/library_sort_menu.gif \
	res/listbox.gif res/listbox_ctrl.gif res/listbox_ctrl_shift.gif res/listbox_shift.gif \
	res/Logo.gif res/mainwnd.gif res/mainwnd_help_menu.gif res/mainwnd_midi_menu.gif \
	res/mainwnd_options_menu.gif res/mainwnd_program_menu.gif \
	res/mainwnd_window_menu.gif res/menu.gif res/midi_keyboard.gif res/midi_setup.gif \
	res/misc.gif res/mtodlg.gif res/multiedit.gif res/multiedit_1.gif \
	res/multiedit_output.gif res/output.gif res/pctable.gif res/pe_assign.gif \
	res/pe_copyinst.gif res/pe_edit_menu.gif res/pe_effect.gif res/pe_help_menu.gif \
	res/pe_inst.gif res/pe_menubtn.gif res/pe_mtkey.gif res/pe_mttable.gif \
	res/pe_pfm_menu.gif res/pe_store_pfm.gif res/pe_window_menu.gif res/pfmdlg.gif \
	res/pitchbias.gif res/pitchmod.gif res/popupmenu.gif res/portamento.gif \
	res/pushbutton.gif res/pushlike.gif res/recommentdlg.gif res/remote_control.gif \
	res/renamedlg.gif res/rev1_0.gif res/rev1_1.gif res/rev1_2.gif res/rev1_3.gif res/rev1_4.gif \
	res/rev1_5.gif res/rev1_6.gif res/rpanel.gif res/scrollbar.gif res/searchdlg.gif \
	res/shortcut1.gif res/shortcut2.gif res/shortcut3.gif res/shortcut4.gif res/shortcut5.gif \
	res/shortcut6.gif res/shortcut7.gif res/shortcut8.gif res/simplefreq.gif res/snapshot.gif \
	res/snapshot_list.gif res/snapshot_menu.gif res/storedlg.gif res/sysdlg.gif \
	res/system_menu.gif res/testentry.gif res/ve_edit_menu.gif res/ve_help_menu.gif \
	res/ve_menubtn_copy.gif res/ve_menubtn_rand.gif res/ve_menubtn_swap.gif \
	res/ve_menubtns.gif res/ve_options_menu.gif res/ve_store_dlg.gif \
	res/ve_voice_menu.gif res/ve_window_menu.gif res/voice_editor.gif res/vol.gif \
	res/window.gif res/styles.css 
OUTPUTPATH=..\Win32\Release
TARGET=$(OUTPUTPATH)\TX81Z.chm

$(TARGET): TX81Z.hhp TX81Z.hhc TX81Z.hhk $(DEPS)
# -1 turns off error checking: https://docs.microsoft.com/en-us/cpp/build/command-modifiers
	-1 $(HHC) TX81Z.hhp

clean:
	if exist $(TARGET) del $(TARGET)
