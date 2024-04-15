extends Button

func _on_file_dialog_confirmed():
	print("LOAD ", $FileDialog.current_file)
	
	var save_game_json := FileAccess.get_file_as_string($FileDialog.current_file)
	var json := JSON.new()
	var error = json.parse(save_game_json)
	if error == OK:
		var save_game : Dictionary = json.data
		restore_save(save_game)
	else:
		print("JSON Parse Error: ", json.get_error_message(), " in ", save_game_json, " at line ", json.get_error_line())
	
func _on_file_dialog_canceled():
	print("SAVE Cancelled")

func _on_pressed():
	$FileDialog.visible = true

func populate_static(save_game : Dictionary, static_instance : Control) -> void:
	if save_game["shape"] == 0:
		static_instance.find_child("CheckRect").button_pressed = 1
	static_instance.find_child("XText").value = save_game["x"]
	static_instance.find_child("YText").value = save_game["y"]
	static_instance.find_child("AngleText").value = save_game["angle"]
	static_instance.find_child("SizeText").selected = save_game["size"] 
	static_instance.find_child("TypeText").selected = save_game["type"]
	
func populate_elliptic(save_game : Dictionary, elliptic_instance : Control) -> void:
	if save_game["shape"] == 0:
		elliptic_instance.find_child("CheckRect").button_pressed = 1
	elliptic_instance.find_child("XText").value = save_game["x"]
	elliptic_instance.find_child("YText").value = save_game["y"]
	elliptic_instance.find_child("AngleText").value = save_game["angle"]
	elliptic_instance.find_child("SizeText").selected = save_game["size"] 
	elliptic_instance.find_child("ArcText").value = save_game["arc"]
	elliptic_instance.find_child("ArcAngleCheckbox").button_pressed = save_game["use_arc"]
	elliptic_instance.find_child("SpeedText").value = save_game["speed"]
	elliptic_instance.find_child("AText").value = save_game["a"]
	elliptic_instance.find_child("BText").value = save_game["b"]
	elliptic_instance.find_child("PathSlider").value = save_game["n_pegs"]
	# Changing PathSliderPathSlider. value will populate the sub-nodes already, just need to set them
	for i in range(0, save_game["n_pegs"]):
		var peg_name : String = "PegContainer"+str(i+1)
		var peg_container : Control = elliptic_instance.find_child(peg_name, true, false)
		var peg_save : Dictionary = save_game[peg_name]
		peg_container.find_child("ShapeButton").selected = peg_save["shape_override"]
		peg_container.find_child("SizeButton").selected = peg_save["size_override"]
		peg_container.find_child("TypeButton").selected = peg_save["type"]
		
func populate_linear(save_game : Dictionary, linear_instance : Control) -> void:
	if save_game["shape"] == 0:
		linear_instance.find_child("CheckRect").button_pressed = 1
	linear_instance.find_child("XText").value = save_game["x"]
	linear_instance.find_child("YText").value = save_game["y"]
	linear_instance.find_child("AngleText").value = save_game["angle"]
	linear_instance.find_child("SizeText").selected = save_game["size"] 
	linear_instance.find_child("ArcText").value = save_game["arc"]
	linear_instance.find_child("ArcAngleCheckbox").button_pressed = save_game["use_arc"]
	linear_instance.find_child("SpeedText").value = save_game["speed"]
	linear_instance.find_child("PathSlider").value = save_game["n_pegs"]
	# Changing PathSliderPathSlider. value will populate the sub-nodes already, just need to set them
	for i in range(0, save_game["n_pegs"]):
		var peg_name : String = "PegContainer"+str(i+1)
		var peg_container : Control = linear_instance.find_child(peg_name, true, false)
		var peg_save : Dictionary = save_game[peg_name]
		peg_container.find_child("ShapeButton").selected = peg_save["shape_override"]
		peg_container.find_child("SizeButton").selected = peg_save["size_override"]
		peg_container.find_child("TypeButton").selected = peg_save["type"]
	linear_instance.find_child("LineSlider").value = save_game["n_lines"]
	for i in range(0, save_game["n_lines"]):
		var line_name : String = "LineContainer"+str(i+1)
		var line_container : Control = linear_instance.find_child(line_name, true, false)
		var line_save : Dictionary = save_game[line_name]
		line_container.find_child("XOffText").value = line_save["lc_x"] - save_game["x"] 
		line_container.find_child("YOffText").value = line_save["lc_y"] - save_game["y"]


func reset_level() -> void:
	var i := 0
	for peg in get_tree().get_nodes_in_group("pegs"):
		peg.name = "temp" + str(i)
		peg.queue_free()
		i += 1

func restore_save(save_game : Dictionary) -> void:
	reset_level()
	
	%Name.text = save_game["header"]["name"] 
	%Author.text = save_game["header"]["author"] 
	%LevelSlider.value = save_game["header"]["level"] 
	%HoleSlider.value = save_game["header"]["hole"]
	%ParSlider.value = save_game["header"]["par"]
	%Foreground.selected = save_game["header"]["foreground"]
	%Background.selected = save_game["header"]["background"]
	%SpecialButton.selected = save_game["header"]["special"]
		
	%LevelSlider._on_value_changed(%LevelSlider.value)
	%HoleSlider._on_value_changed(%HoleSlider.value)
	%ParSlider._on_value_changed(%ParSlider.value)
	
	var static_pegs : int = save_game["header"]["n_static"]
	var elliptic_paths : int = save_game["header"]["n_elliptic"]
	var linear_paths : int = save_game["header"]["n_linear"]
	
	%AddStatic.reset_count()
	for i in range(1, static_pegs+1):
		%AddStatic._on_pressed()
		var static_name = "StaticControl" + str(i)
		var static_instance = %RightVBox.find_child(static_name, true, false)
		populate_static(save_game["body"][static_name], static_instance)
		
	%AddElliptic.reset_count()
	for i in range(1, elliptic_paths+1):
		%AddElliptic._on_pressed()
		var elliptic_name = "EllipticControl" + str(i)
		var elliptic_instance = %RightVBox.find_child(elliptic_name, true, false)
		populate_elliptic(save_game["body"][elliptic_name], elliptic_instance)
		
	%AddLinear.reset_count()
	for i in range(1, linear_paths+1):
		%AddLinear._on_pressed()
		var linear_name = "LinearControl" + str(i)
		var linear_instance = %RightVBox.find_child(linear_name, true, false)
		populate_linear(save_game["body"][linear_name], linear_instance)
