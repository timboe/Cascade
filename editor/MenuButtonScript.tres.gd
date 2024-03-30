extends Control

@onready var rend = $/root/RootControl.find_child("Render")

func _on_pressed():
	rend.do_update()


func _on_toggled(toggled_on):
	$"../AngleText".editable = !toggled_on
