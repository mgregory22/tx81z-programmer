HHCPATH=C:\Util
OUTPUTPATH=.
HHC=$(HHCPATH)\hhc.exe
DEPS=algctrl.html algorithm.html ampmod.html bundles.html buttons.html \
	comboboxes.html controls.html dialogs.html edit_buffers.html \
	editable_items.html effects_editor.html eg.html egbias.html \
	frequency.html glossary.html index.html invokation.html \
	item_types.html keyboard_navigation.html lcdctrl.html lfo.html \
	libraries.html library_area.html library_buttons.html \
	library_menus.html license.html listboxes.html mainwnd.html \
	mainwnd_controls.html mainwnd_menu.html menus.html \
	microtunings_editors.html midi_keyboard.html misc.html mysteries.html \
	note.html output.html pctable_editor.html \
	pe_controls.html pe_menus.html performance_editor.html pitchbias.html \
	pitchmod.html portamento.html quickstart.html remote_control.html \
	rpanel.html scrollbars.html snapshot.html snapshot_area.html \
	snapshot_buttons.html snapshot_files.html snapshot_menu.html \
	synchronization.html system_editor.html template.html \
	tx81z_notes.html ve_eggraphs.html ve_lcdctrls.html ve_menubtns.html \
	ve_menus.html ve_multiedit.html ve_scrolling.html ve_simplefreq.html \
	voice_editor.html vol.html windows.html algctrl.gif algorithm.gif \
	ampmod.gif bndldlg.gif checkbox.gif combobox.gif datadlg.gif \
	dataflow.gif dialog.gif diff.gif editkeys.gif eg.gif egbias.gif \
	EGgraphs.gif excel.gif exportdlg.gif frequency.gif fxdlg.gif \
	importdlg.gif keynav.gif lcdctrl.gif lfo.gif library_area.gif \
	library_buttons.gif library_file_menu.gif library_sort_menu.gif \
	listbox.gif listbox_ctrl.gif listbox_ctrl_shift.gif listbox_shift.gif \
	Logo.gif mainwnd.gif mainwnd_help_menu.gif mainwnd_midi_menu.gif \
	mainwnd_options_menu.gif mainwnd_program_menu.gif \
	mainwnd_window_menu.gif menu.gif midi_keyboard.gif midi_setup.gif \
	misc.gif mtodlg.gif multiedit.gif multiedit_1.gif \
	multiedit_output.gif output.gif pctable.gif pe_assign.gif \
	pe_copyinst.gif pe_edit_menu.gif pe_effect.gif pe_help_menu.gif \
	pe_inst.gif pe_menubtn.gif pe_mtkey.gif pe_mttable.gif \
	pe_pfm_menu.gif pe_store_pfm.gif pe_window_menu.gif pfmdlg.gif \
	pitchbias.gif pitchmod.gif popupmenu.gif portamento.gif \
	pushbutton.gif pushlike.gif recommentdlg.gif remote_control.gif \
	renamedlg.gif rev1_0.gif rev1_1.gif rev1_2.gif rev1_3.gif rev1_4.gif \
	rev1_5.gif rev1_6.gif rpanel.gif scrollbar.gif searchdlg.gif \
	shortcut1.gif shortcut2.gif shortcut3.gif shortcut4.gif shortcut5.gif \
	shortcut6.gif shortcut7.gif shortcut8.gif simplefreq.gif snapshot.gif \
	snapshot_list.gif snapshot_menu.gif storedlg.gif sysdlg.gif \
	system_menu.gif testentry.gif ve_edit_menu.gif ve_help_menu.gif \
	ve_menubtn_copy.gif ve_menubtn_rand.gif ve_menubtn_swap.gif \
	ve_menubtns.gif ve_options_menu.gif ve_store_dlg.gif \
	ve_voice_menu.gif ve_window_menu.gif voice_editor.gif vol.gif \
	window.gif styles.css 

$(OUTPUTPATH)\TX81Z.chm: TX81Z.hhp TX81Z.hhc TX81Z.hhk $(DEPS)
	-1 $(HHC) TX81Z.hhp
