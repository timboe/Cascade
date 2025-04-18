extends Node2D

const BALL_RADIUS : float = 8.0
const GRAB_CIRCLE_RADIUS : float = BALL_RADIUS/2
const PLAYDATE_WIDTH : int = 400

var dragMode : int = 0
var dragNode : Control = null
var dragPos : Vector2

var mousePos : Vector2

var mostRecentlyCloned

func find_peg(v : Vector2, n : String) -> Control:
	for peg in get_tree().get_nodes_in_group(n):
		var v2 := Vector2()
		v2.x = peg.find_child("XText").value
		v2.y = peg.find_child("YText").value
		if v.distance_to(v2) < GRAB_CIRCLE_RADIUS:
			return peg
	return null
	
func find_line_segment(v : Vector2, n : String) -> Control:
	for peg in get_tree().get_nodes_in_group(n):
		var v1 := Vector2()
		v1.x = peg.find_parent("LinearControl*").find_child("XText").value
		v1.y = peg.find_parent("LinearControl*").find_child("YText").value
		var v2 := Vector2()
		v2.x = peg.find_child("XOffText").value
		v2.y = peg.find_child("YOffText").value
		v1 += v2
		if v.distance_to(v1) < GRAB_CIRCLE_RADIUS:
			return peg
	return null
	
func move_peg(peg : Control, v : Vector2) -> void:
	var x_text = peg.find_child("XText")
	var y_text = peg.find_child("YText")
	var x_off_text = peg.find_child("XOffText")
	var y_off_text = peg.find_child("YOffText")
	if x_text and y_text:
		x_text.value = v.x
		y_text.value = v.y
	elif x_off_text and y_off_text:
		var parent_x = peg.find_parent("LinearControl*").find_child("XText").value
		var parent_y = peg.find_parent("LinearControl*").find_child("YText").value
		x_off_text.value = v.x - parent_x
		y_off_text.value = v.y - parent_y
	
func rotate_peg(peg : Control, delta : float) -> void:
	if not peg: return
	var angle = peg.find_child("AngleText")
	if not angle: return
	angle.value += delta

func set_scrollbottom():
	await get_tree().process_frame 
	%RightScroll.scroll_vertical = %RightScroll.get_v_scroll_bar().max_value

func set_scroll(peg):
	for peg_remove : Control in get_tree().get_nodes_in_group("pegs"):
		peg_remove.remove_theme_stylebox_override("panel")
	var parentLineControl = peg.find_parent("LinearControl*")
	#print("Found " , peg, " with parent ", parentLineControl, " at y ",  parentLineControl.position.y)
	if parentLineControl:
		parentLineControl.add_theme_stylebox_override("panel", load("res://selected_style_box_flat.tres"))
		%RightScroll.set_v_scroll( parentLineControl.position.y )
		print("LSSto " , parentLineControl.position.y)
	else:
		peg.add_theme_stylebox_override("panel", load("res://selected_style_box_flat.tres"))
		%RightScroll.set_v_scroll( peg.position.y )
		print("PSSto " , peg.position.y)
		
func _input(event):
	if event is InputEventKey:
		if event.pressed and dragNode:
			var snap = %EditorSnap.value
			if event.keycode == KEY_UP :
				var n = dragNode.find_child("YText", true, false)
				if n:
					n.value -= snap
					get_tree().get_root().set_input_as_handled()
			if event.keycode == KEY_DOWN :
				var n = dragNode.find_child("YText", true, false)
				if n:
					n.value += snap
					get_tree().get_root().set_input_as_handled()
			if event.keycode == KEY_LEFT :
				var n = dragNode.find_child("XText", true, false)
				if n:
					n.value -= snap
					get_tree().get_root().set_input_as_handled()
			if event.keycode == KEY_RIGHT :
				var n = dragNode.find_child("XText", true, false)
				if n:
					n.value += snap
					get_tree().get_root().set_input_as_handled()
			if event.keycode == KEY_C :
				var searchFrom = dragNode.find_parent("LinearControl*")
				if not searchFrom:
					searchFrom = dragNode
				var clone = searchFrom.find_child("Clone", true, false) if searchFrom else null
				if clone:
					clone._on_pressed()
					get_tree().get_root().set_input_as_handled()
					dragNode = mostRecentlyCloned
					set_scroll(dragNode)
				else:
					print("No Clone button for ", dragNode, " ", clone)
			if event.keycode == KEY_DELETE :
				var toRemove = dragNode.find_child("Remove", true, false)
				if toRemove:
					toRemove._on_pressed()
					get_tree().get_root().set_input_as_handled()
					dragNode = null
					
	if event is InputEventMouseButton:
		if (event.button_index == 1 || event.button_index == 2) and event.pressed and event.position.x < PLAYDATE_WIDTH:
			var yOff = %LeftScroll.get_v_scroll()
			var v = Vector2(event.position.x, event.position.y + yOff)
			print("Mouse " , event.button_index , " Click/Unclick at: ", v)

			var peg : Control = find_peg(v, "static_pegs")
			if not peg: peg = find_peg(v, "elliptic_pegs")
			if not peg: peg = find_peg(v, "line_pegs")
			if peg:
				print("Found " , peg)
				set_scroll(peg)
				dragMode = event.button_index
				dragNode = peg
				dragPos = v
				return
			if not peg: peg = find_line_segment(v, "line_segments")
			if peg:
				set_scroll(peg)
				dragMode = event.button_index
				dragNode = peg
				dragPos = v
				return
			dragNode = null
		elif (event.button_index == 1 || event.button_index == 2) and not event.pressed:
			dragMode = 0
			#dragNode = null

	if event is InputEventMouseMotion:
		var yOff = $%LeftScroll.get_v_scroll()
		var v = Vector2(event.position.x, event.position.y + yOff)
		v.x = snap(v.x)
		v.y = snap(v.y)
		mousePos = v
		if dragMode:
			print("Drag ", v)
			if dragMode == 1: 
				move_peg(dragNode, v)
			elif dragMode == 2:
				if abs(v.y - dragPos.y) > abs(v.x - dragPos.x):
					rotate_peg(dragNode, v.y - dragPos.y)
				else:
					rotate_peg(dragNode, v.x - dragPos.x)
				dragPos = v
		
func snap(v):
	return round( v / %EditorSnap.value ) * %EditorSnap.value
