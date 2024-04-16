extends Control

@onready var rend = $/root/RootControl.find_child("Render")

func _on_pressed():
	rend.do_update()

func _on_value_changed(value):
	rend.do_update()
