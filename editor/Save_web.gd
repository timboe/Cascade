extends Button

var text_modal_js_path: String = "res://native/js/text_modal.js"
var text_modal_js: String = read_text_from(text_modal_js_path)

static func read_text_from(path: String) -> String:
	var file: FileAccess = FileAccess.open(path, FileAccess.READ)
	if file == null:
		return ""
	var content: String = file.get_as_text()
	file.close()
	return content
	
func _get_native_js_popup_command(input: String, atitle: String, subtitle: String, button: String) -> String:
	return text_modal_js.format(
		{"input": input, "atitle": atitle, "subtitle": subtitle, "button": button}
	)

func _await_native_js_popup_command(input: String, atitle: String, subtitle: String, button: String) -> String:
	var command: String = _get_native_js_popup_command(input, atitle, subtitle, button)
	var eval_return: Variant = JavaScriptBridge.eval(command)
	while true:
		eval_return = JavaScriptBridge.eval("window.globalTextAreaResult")
		if eval_return != null:
			break
		await get_tree().create_timer(0.1).timeout
	JavaScriptBridge.eval("window.globalTextAreaResult = null;")
	return eval_return

func _on_Export_pressed():
	print("Doing export")
	var save_game = pack_save()
	print("Save packed")
	var level : int = $"../LevelSlider".value
	var hole : int = $"../HoleSlider".value
	var save_string = JSON.stringify(save_game, "", false)
	print("Save JSONd")
	var save_level = String("round_"+str(level)+"_hole_"+str(hole)+".json")
	print("Show dialogue to save " + save_level)
	print(save_string)

	await _await_native_js_popup_command(
		save_string,
		"Export Hole",
		"Save the following JSON as: " + save_level,
		"OK"
	)


func pack_static(static_peg : Control) -> Dictionary:
	var peg_name : String = static_peg.name
	var payload : Dictionary
	var shape
	if static_peg.find_child("CheckRect").button_pressed:
		shape = 0
	elif static_peg.find_child("CheckCirc").button_pressed:
		shape = 1
	elif static_peg.find_child("CheckTriangle").button_pressed:
		shape = 2
	payload["shape"] = shape
	payload["x"] = static_peg.find_child("XText").value
	payload["y"] = static_peg.find_child("YText").value
	payload["angle"] = static_peg.find_child("AngleText").value
	payload["size"] = static_peg.find_child("SizeText").selected
	payload["type"] = static_peg.find_child("TypeText").selected
	return payload
	
func pack_elliptic(elliptic_path : Control) -> Dictionary:
	var peg_name : String = elliptic_path.name
	var payload : Dictionary
	var shape
	if elliptic_path.find_child("CheckRect").button_pressed:
		shape = 0
	elif elliptic_path.find_child("CheckCirc").button_pressed:
		shape = 1
	elif elliptic_path.find_child("CheckTriangle").button_pressed:
		shape = 2
	payload["shape"] = shape
	payload["x"] = elliptic_path.find_child("XText").value
	payload["y"] = elliptic_path.find_child("YText").value
	payload["angle"] = elliptic_path.find_child("AngleText").value
	payload["size"] = elliptic_path.find_child("SizeText").selected
	payload["arc"] = elliptic_path.find_child("ArcText").value
	payload["use_arc"] = elliptic_path.find_child("ArcAngleCheckbox").button_pressed
	payload["speed"] = elliptic_path.find_child("SpeedText").value
	payload["easing"] = elliptic_path.find_child("EaseText").selected
	payload["a"] = elliptic_path.find_child("AText").value
	payload["b"] = elliptic_path.find_child("BText").value
	payload["n_pegs"] = elliptic_path.find_child("PathSlider").value
	for i in range(0, payload["n_pegs"]):
		var peg_payload : Dictionary
		var peg_container : Control = elliptic_path.find_child("PegContainer"+str(i+1), true, false)
		peg_payload["shape_override"] = peg_container.find_child("ShapeButton").selected
		peg_payload["size_override"] = peg_container.find_child("SizeButton").selected
		peg_payload["type"] = peg_container.find_child("TypeButton").selected
		payload[peg_container.name] = peg_payload
	return payload

func pack_linear(linear_path : Control) -> Dictionary:
	var peg_name : String = linear_path.name
	var payload : Dictionary
	var shape
	if linear_path.find_child("CheckRect").button_pressed:
		shape = 0
	elif linear_path.find_child("CheckCirc").button_pressed:
		shape = 1
	elif linear_path.find_child("CheckTriangle").button_pressed:
		shape = 2
	payload["shape"] = shape
	payload["x"] = linear_path.find_child("XText").value
	payload["y"] = linear_path.find_child("YText").value
	payload["angle"] = linear_path.find_child("AngleText").value
	payload["size"] = linear_path.find_child("SizeText").selected
	payload["arc"] = linear_path.find_child("ArcText").value
	payload["use_arc"] = linear_path.find_child("ArcAngleCheckbox").button_pressed
	payload["speed"] = linear_path.find_child("SpeedText").value
	payload["easing"] = linear_path.find_child("EaseText").selected
	payload["n_pegs"] = linear_path.find_child("PathSlider").value
	payload["n_lines"] = linear_path.find_child("LineSlider").value
	for i in range(0, payload["n_pegs"]):
		var peg_payload : Dictionary
		var peg_container : Control = linear_path.find_child("PegContainer"+str(i+1), true, false)
		peg_payload["shape_override"] = peg_container.find_child("ShapeButton").selected
		peg_payload["size_override"] = peg_container.find_child("SizeButton").selected
		peg_payload["type"] = peg_container.find_child("TypeButton").selected
		payload[peg_container.name] = peg_payload
	for i in range(0, payload["n_lines"]):
		var line_payload : Dictionary
		var line_container : Control = linear_path.find_child("LineContainer"+str(i+1), true, false)
		line_payload["lc_x"] = payload["x"] + line_container.find_child("XOffText").value
		line_payload["lc_y"] = payload["y"] + line_container.find_child("YOffText").value
		payload[line_container.name] = line_payload
	return payload

func pack_save() -> Dictionary:
	var save_game : Dictionary
	var header : Dictionary
	header["save_format"] = 1
	header["name"] = %Name.text
	header["author"] = %Author.text
	header["level"] = %LevelSlider.value
	header["hole"] = %HoleSlider.value
	header["par"] = %ParSlider.value
	header["foreground"] = %Foreground.get_item_id(%Foreground.selected) + 1
	header["background"] = %Background.get_item_id(%Background.selected) + 1
	header["special"] = %SpecialButton.selected
	header["height"] = %HeightSlider.value
	header["snap"] = %EditorSnap.value
	
	var static_pegs = get_tree().get_nodes_in_group("static_pegs")
	var elliptic_paths = get_tree().get_nodes_in_group("elliptic_pegs")
	var line_paths = get_tree().get_nodes_in_group("line_pegs")
	
	save_game["header"] = header
	
	var body : Dictionary
	var counter = 0
	for static_peg in static_pegs:
		body["StaticControl"+str(counter+1)] = pack_static(static_peg)
		counter += 1
	header["n_static"] = counter
	
	counter = 0
	for elliptic_path in elliptic_paths:
		body["EllipticControl"+str(counter+1)] = pack_elliptic(elliptic_path)
		counter += 1
	header["n_elliptic"] = counter
		
	counter = 0
	for linear_path in line_paths:
		body["LinearControl"+str(counter+1)] = pack_linear(linear_path)
		counter += 1
	header["n_linear"] = counter
	
	save_game["body"] = body

	return save_game
