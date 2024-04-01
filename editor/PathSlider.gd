extends HSlider

@onready var pegScene = load("res://peg_container.tscn")
@onready var lineScene = load("res://line_container.tscn")
@onready var rend = $/root/RootControl.find_child("Render")
@onready var parentControl : Control = get_parent().get_parent().get_parent().get_parent()

var total_peg = 1
var total_line = 1

func get_y_size() -> int:
	if "LineControl" in parentControl.name:
		print("resize for pegs ", total_peg," line segments ", total_line)
		return 340 + ((total_peg+1) * (27+6)) + ((total_line+1) * (27+6))
	return 320 + ((total_peg+1) * (27+6))

func _on_value_changed(newValue):
	$"../PathLabel".text = "Path Pegs: " + str(newValue)
	rend.do_update()
	var pegVbox : VBoxContainer = parentControl.find_child("PegVBox")
	total_peg = newValue
	parentControl.custom_minimum_size.y = get_y_size()
	for i in range(1, newValue+1):
		var pName = "PegContainer"+str(i)
		var pegControl = pegVbox.find_child(pName,true,false)
		if pegControl == null:
			print("add peg")
			var instance = pegScene.instantiate()
			instance.name = pName
			pegVbox.add_child(instance)
			instance.find_child("PegLabel").text = str(i)
	for i in range(newValue+1, max_value+1):
		var pName = "PegContainer"+str(i)
		var pegControl = pegVbox.find_child(pName,true,false)
		#print("look for ",pName," in ", ellipticVbox," got " , pegControl)
		if pegControl != null:
			pegControl.queue_free()


func _on_line_slider_value_changed(newValue):
	$"../LineLabel".text = "Path Segments:" + str(newValue)
	rend.do_update()
	var lineVbox : VBoxContainer = parentControl.find_child("LineVBox")
	total_line = newValue
	parentControl.custom_minimum_size.y = get_y_size()
	for i in range(1, newValue+1):
		var pName = "LineContainer"+str(i)
		var lineControl = lineVbox.find_child(pName,true,false)
		if lineControl == null:
			print("add line to ",lineVbox)
			var instance = lineScene.instantiate()
			instance.name = pName
			lineVbox.add_child(instance)
			instance.find_child("LineLabel").text = str(i)
			instance.find_child("XOffText").value = 16
			instance.find_child("YOffText").value = 16*(i-1)
	for i in range(newValue+1, max_value+1):
		var pName = "LineContainer"+str(i)
		var lineControl = lineVbox.find_child(pName,true,false)
		if lineControl != null:
			lineControl.queue_free()
