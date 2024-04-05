extends Button

func _on_pressed():
	var world : int = $"../WorldSlider".value
	var level : int = $"../LevelSlider".value
	var save_name := String("fall_" + str(world) + "_hole_" + str(level) + ".json")
	$FileDialog.current_file = save_name
	$FileDialog.visible = true

func _on_file_dialog_confirmed():
	var save_name = $FileDialog.current_path
	var save_game = pack_save()
	var file = FileAccess.open(save_name, FileAccess.WRITE)
	var save_game_json = JSON.stringify(save_game, "  ", false)
	file.store_line(save_game_json)
	file.close()
	print("SAVE ", save_name)
	print("SAVE ", save_game_json)
	
func _on_file_dialog_canceled():
	print("SAVE Cancelled")

func pack_static(static_peg : Control) -> Array:
	var peg_name : String = static_peg.name
	var payload : Dictionary
	payload["shape"] = 1 if static_peg.find_child("CheckCirc").button_pressed else 0
	payload["x"] = static_peg.find_child("XText").value
	payload["y"] = static_peg.find_child("YText").value
	payload["angle"] = static_peg.find_child("AngleText").value
	payload["size"] = static_peg.find_child("SizeText").selected
	payload["type"] = static_peg.find_child("TypeText").selected
	return [peg_name, payload]
	
func pack_elliptic(elliptic_path : Control) -> Array:
	var peg_name : String = elliptic_path.name
	var payload : Dictionary
	payload["shape"] = 1 if elliptic_path.find_child("CheckCirc").button_pressed else 0
	payload["x"] = elliptic_path.find_child("XText").value
	payload["y"] = elliptic_path.find_child("YText").value
	payload["angle"] = elliptic_path.find_child("AngleText").value
	payload["size"] = elliptic_path.find_child("SizeText").selected
	payload["arc"] = elliptic_path.find_child("ArcText").value
	payload["use_arc"] = elliptic_path.find_child("ArcAngleCheckbox").button_pressed
	payload["speed"] = elliptic_path.find_child("SpeedText").value
	payload["a"] = elliptic_path.find_child("AText").value
	payload["b"] = elliptic_path.find_child("BText").value
	payload["n_pegs"] = elliptic_path.find_child("PathSlider").value
	for i in range(0, payload["n_pegs"]):
		var peg_payload : Dictionary
		var peg_container : Control = elliptic_path.find_child("PegContainer"+str(i+1), true, false)
		peg_payload["shape"] = peg_container.find_child("ShapeButton").selected
		peg_payload["size"] = peg_container.find_child("SizeButton").selected
		peg_payload["type"] = peg_container.find_child("TypeButton").selected
		payload[peg_container.name] = peg_payload
	return [peg_name, payload]

func pack_linear(linear_path : Control) -> Array:
	var peg_name : String = linear_path.name
	var payload : Dictionary
	payload["shape"] = 1 if linear_path.find_child("CheckCirc").button_pressed else 0
	payload["x"] = linear_path.find_child("XText").value
	payload["y"] = linear_path.find_child("YText").value
	payload["angle"] = linear_path.find_child("AngleText").value
	payload["size"] = linear_path.find_child("SizeText").selected
	payload["arc"] = linear_path.find_child("ArcText").value
	payload["use_arc"] = linear_path.find_child("ArcAngleCheckbox").button_pressed
	payload["speed"] = linear_path.find_child("SpeedText").value
	payload["n_pegs"] = linear_path.find_child("PathSlider").value
	payload["n_lines"] = linear_path.find_child("LineSlider").value
	for i in range(0, payload["n_pegs"]):
		var peg_payload : Dictionary
		var peg_container : Control = linear_path.find_child("PegContainer"+str(i+1), true, false)
		var custom_shape = peg_container.find_child("ShapeButton").selected
		if custom_shape == 0: custom_shape = payload["shape"]
		else: custom_shape -= 1 
		peg_payload["shape"] = custom_shape
		var custom_size = peg_container.find_child("SizeButton").selected
		if custom_size == 0: custom_size = payload["size"]
		else: custom_size -= 1 
		peg_payload["size"] = custom_size
		peg_payload["type"] = peg_container.find_child("TypeButton").selected
		payload[peg_container.name] = peg_payload
	for i in range(0, payload["n_lines"]):
		var line_payload : Dictionary
		var line_container : Control = linear_path.find_child("LineContainer"+str(i+1), true, false)
		line_payload["x"] = payload["x"] + line_container.find_child("XOffText").value
		line_payload["y"] = payload["y"] + line_container.find_child("YOffText").value
		payload[line_container.name] = line_payload
	return [peg_name, payload]

func pack_save() -> Dictionary:
	var save_game : Dictionary
	save_game["author"] = $"../Author".text
	save_game["world"] = $"../WorldSlider".value
	save_game["level"] = $"../LevelSlider".value
	save_game["par"] = $"../ParSlider".value
	save_game["save_format"] = 1
	
	var static_pegs = get_tree().get_nodes_in_group("static_pegs")
	var elliptic_paths = get_tree().get_nodes_in_group("elliptic_pegs")
	var line_paths = get_tree().get_nodes_in_group("line_pegs")
	
	save_game["n_static"] = len(static_pegs)
	save_game["n_elliptic"] = len(elliptic_paths)
	save_game["n_linear"] = len(line_paths)
	
	for static_peg in static_pegs:
		var packed : Array = pack_static(static_peg)
		save_game[packed[0]] = packed[1]
	for elliptic_path in elliptic_paths:
		var packed : Array = pack_elliptic(elliptic_path)
		save_game[packed[0]] = packed[1]
	for linear_path in line_paths:
		var packed : Array = pack_linear(linear_path)
		save_game[packed[0]] = packed[1]
		
	return save_game
