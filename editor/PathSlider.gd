extends HSlider

@onready var pegScene = load("res://peg_container.tscn")
@onready var rend = $/root/RootControl.find_child("Render")
@onready var ellipticControl : Control = get_parent().get_parent().get_parent().get_parent()

func _on_ready():
	ellipticControl.custom_minimum_size.y = 310 + (1 * (31+6))
	
func _on_value_changed(newValue):
	$"../PathLabel".text = "Path Pegs: " + str(newValue)
	rend.do_update()
	var ellipticVbox : VBoxContainer = ellipticControl.find_child("EllipticVBox")
	ellipticControl.custom_minimum_size.y = 310 + ((newValue+1) * (31+6))
	for i in range(1, newValue+1):
		var pName = "PegContainer"+str(i)
		var pegControl = ellipticVbox.find_child(pName,true,false)
		if pegControl == null:
			print("add peg")
			var instance = pegScene.instantiate()
			instance.name = pName
			ellipticVbox.add_child(instance)
			instance.find_child("PegLabel").text = str(i)
	for i in range(newValue+1, max_value+1):
		var pName = "PegContainer"+str(i)
		var pegControl = ellipticVbox.find_child(pName,true,false)
		#print("look for ",pName," in ", ellipticVbox," got " , pegControl)
		if pegControl != null:
			print("rm peg")
			pegControl.queue_free()
