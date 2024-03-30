extends Node2D

const BALL_RADIUS : float = 8.0
const BOX_WIDTH : float = 22.5
const BOX_HEIGHT : float = 12.5
const LINE_WIDTH : float = 2.0

const GRAB_CIRCLE_RADIUS : float = BALL_RADIUS/2

const PLAYDATE_WIDTH : int = 400

var dragMode : int = 0
var dragNode : Control = null
var dragPos : Vector2

var t : float = 0

func sizeToScale(size : int) -> float:
	match size:
		0: return 1.0
		1: return 1.25
		2: return 1.5
		3: return 2.0
	return 1.0;
	
func fillColor(type : int) -> Color:
	match type:
		0: return Color.GRAY
		1: return Color.LIME_GREEN
		2: return Color.SLATE_BLUE
	return Color.WHITE
	
func lineColor(type : int) -> Color:
	match type:
		0: return Color.BLACK
		1: return Color.DARK_GREEN
		2: return Color.NAVY_BLUE
	return Color.BLACK
	
func renderPeg(circ : bool, x : int, y : int, a : float, size : int, type : int) -> void:
	var s := sizeToScale(size)
	if circ: renderCirc(x, y, s, type)
	else: renderRect(x, y, a, s, type)
	
func renderCirc(x : int, y : int, s : float, type : int) -> void:
	draw_circle(Vector2(x, y), BALL_RADIUS*s, fillColor(type))
	draw_arc(Vector2(x, y), BALL_RADIUS*s, 0, 2*PI, 128, lineColor(type), LINE_WIDTH)
	
func renderRect(x : int, y : int, a : float, s : float, type : int) -> void:
	var ca := cos(a)
	var sa := sin(a)
	var w := BOX_WIDTH/2 * s
	var h := BOX_HEIGHT/2 * s
	
	var points : PackedVector2Array = [
		Vector2(
			-w*ca - -h*sa + x,
			-w*sa + -h*ca + y),
		Vector2(
			-w*ca - h*sa + x,
			-w*sa + h*ca + y),
		Vector2(
			w*ca - h*sa + x,
			w*sa + h*ca + y),
		Vector2(
			w*ca - -h*sa + x,
			w*sa + -h*ca + y),
		Vector2(
			-w*ca - -h*sa + x,
			-w*sa + -h*ca + y),
	]
		
	draw_polygon(points, [fillColor(type)])
	draw_polyline(points, lineColor(type), 4)

func overlay_fill_color(sel : bool):
	if sel: return Color.CADET_BLUE
	return Color.RED
	
func overlay_line_color(sel : bool):
	if sel: return Color.DARK_SLATE_BLUE
	return Color.DARK_RED

func render_grab_shape(x : int, y : int, sel : bool) -> void:
	draw_circle(Vector2(x, y), GRAB_CIRCLE_RADIUS, overlay_fill_color(sel))
	draw_arc(Vector2(x, y), GRAB_CIRCLE_RADIUS, 0, 2*PI, 128, overlay_line_color(sel), LINE_WIDTH)
	
func render_line_path_grab_shapes(line_path_array : Array, sel : bool) -> void:
	for i in range(1, len(line_path_array)):
		draw_line (line_path_array[i-1], line_path_array[i],overlay_line_color(sel), LINE_WIDTH)
		if i != len(line_path_array)-1:
			var rect := Rect2(line_path_array[i] - Vector2(GRAB_CIRCLE_RADIUS, GRAB_CIRCLE_RADIUS), Vector2(2*GRAB_CIRCLE_RADIUS, 2*GRAB_CIRCLE_RADIUS))
			draw_rect (rect, overlay_fill_color(sel), true)
			draw_rect (rect, overlay_line_color(sel), false, LINE_WIDTH)

func render_static_peg(staticPeg : Control):
	var circ : bool = staticPeg.find_child("CheckCirc").button_pressed
	var x : int = staticPeg.find_child("XText").value
	var y : int = staticPeg.find_child("YText").value
	var angle : int = staticPeg.find_child("AngleText").value
	var angle_rad : float =  (2*PI / 360.0) * angle
	var size : int = staticPeg.find_child("SizeText").selected
	var type : int = staticPeg.find_child("TypeText").selected
	renderPeg(circ, x, y, angle_rad, size, type)
	render_grab_shape(x, y, staticPeg == dragNode)

func render_elliptic_path(ellipticPath : Control):
	var n_pegs : int = ellipticPath.find_child("PathSlider").value
	var circ : bool = ellipticPath.find_child("CheckCirc").button_pressed
	var size : int = ellipticPath.find_child("SizeText").selected
	var ease : int = ellipticPath.find_child("EaseText").selected
	var x : int = ellipticPath.find_child("XText").value
	var y : int = ellipticPath.find_child("YText").value
	var arc : int = ellipticPath.find_child("ArcText").value
	var arc_rad : float =  (2*PI / 360.0) * arc
	var speed : float = ellipticPath.find_child("SpeedText").value
	var a : int = ellipticPath.find_child("AText").value
	var b : int = ellipticPath.find_child("BText").value
	var angle : int = ellipticPath.find_child("AngleText").value
	var angle_rad : float =  (2*PI / 360.0) * angle
	var use_arc : int = ellipticPath.find_child("ArcAngleCheckbox").button_pressed
	
	for i in range(0, n_pegs):
		var peg_container : Control = ellipticPath.find_child("PegContainer"+str(i+1), true, false)
		var custom_shape = peg_container.find_child("ShapeButton").selected
		var custom_size = peg_container.find_child("SizeButton").selected
		var custom_type = peg_container.find_child("TypeButton").selected
		var shape_peg = circ
		var size_peg = size
		if custom_shape:
			shape_peg = 1 if custom_shape == 1 else 0
		if custom_size:
			size_peg = custom_size-1
		var angle_peg : float = (arc_rad / n_pegs) * i
		var x_peg : float = x + (a * cos((t * speed) + angle_peg))
		var y_peg : float = y + (b * sin((t * speed) + angle_peg))
		var draw_angle : float
		if use_arc:
			draw_angle = (t * speed) + angle_peg + (PI * 0.5)
		else:
			draw_angle = angle_rad
		renderPeg(shape_peg, x_peg, y_peg, draw_angle, size_peg, custom_type)
	render_grab_shape(x, y, ellipticPath == dragNode)
	
func render_line_path(linePath : Control):
	var my_nodes : Array
	my_nodes.push_back(linePath)
	
	var n_lines : int = linePath.find_child("LineSlider").value
	var n_pegs : int = linePath.find_child("PathSlider").value
	var circ : bool = linePath.find_child("CheckCirc").button_pressed
	var size : int = linePath.find_child("SizeText").selected
	var ease : int = linePath.find_child("EaseText").selected
	var x : int = linePath.find_child("XText").value
	var y : int = linePath.find_child("YText").value
	var arc : int = linePath.find_child("ArcText").value
	var arc_rad : float =  (2*PI / 360.0) * arc
	var speed : float = linePath.find_child("SpeedText").value
	var angle : int = linePath.find_child("AngleText").value
	var angle_rad : float =  (2*PI / 360.0) * angle
	var use_arc : int = linePath.find_child("ArcAngleCheckbox").button_pressed
	
	var line_path_array : Array
	line_path_array.push_back(Vector2(x,y))
	for i in range(0, n_lines):
		var line_container : Control = linePath.find_child("LineContainer"+str(i+1), true, false)
		my_nodes.push_back(line_container)
		var x_off = line_container.find_child("XOffText").value
		var y_off = line_container.find_child("YOffText").value
		line_path_array.push_back(Vector2(x+x_off, y+y_off))
	line_path_array.push_back(Vector2(x,y))
	

	
	#for i in range(0, n_pegs):
		#var peg_container : Control = ellipticPath.find_child("PegContainer"+str(i+1), true, false)
		#var custom_shape = peg_container.find_child("ShapeButton").selected
		#var custom_size = peg_container.find_child("SizeButton").selected
		#var custom_type = peg_container.find_child("TypeButton").selected
		#var shape_peg = circ
		#var size_peg = size
		#if custom_shape:
			#shape_peg = 1 if custom_shape == 1 else 0
		#if custom_size:
			#size_peg = custom_size-1
		#var angle_peg : float = (arc_rad / n_pegs) * i
		#var x_peg : float = x + (a * cos((t * speed) + angle_peg))
		#var y_peg : float = y + (b * sin((t * speed) + angle_peg))
		#var draw_angle : float
		#if use_arc:
			#draw_angle = (t * speed) + angle_peg + (PI * 0.5)
		#else:
			#draw_angle = angle_rad
		#renderPeg(shape_peg, x_peg, y_peg, draw_angle, size_peg, custom_type)
	render_line_path_grab_shapes(line_path_array, dragNode in my_nodes)
	render_grab_shape(x, y, dragNode in my_nodes)
	
#
func _process(delta):
	t += delta
	queue_redraw()
	
func do_update():
	#print("Ping for redraw")
	queue_redraw()

func do_update_value(_value):
	do_update()

func _draw():
	var staticPegs = get_tree().get_nodes_in_group("static_pegs")
	var ellipticPaths = get_tree().get_nodes_in_group("elliptic_pegs")
	var linePaths = get_tree().get_nodes_in_group("line_pegs")
	#print("Redraw ", len(staticPegs) , " static")
	for staticPeg in staticPegs:
		render_static_peg(staticPeg)
	for ellipticPath in ellipticPaths:
		render_elliptic_path(ellipticPath)
	for linePath in linePaths:
		render_line_path(linePath)
				
	draw_line(
		Vector2(0, $%HeightSlider.value), Vector2(PLAYDATE_WIDTH, $%HeightSlider.value),
	 	Color.RED, LINE_WIDTH*2)

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
		v1.x = peg.find_parent("LineControl").find_child("XText").value
		v1.y = peg.find_parent("LineControl").find_child("YText").value
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
		var parent_x = peg.find_parent("LineControl").find_child("XText").value
		var parent_y = peg.find_parent("LineControl").find_child("YText").value
		x_off_text.value = v.x - parent_x
		y_off_text.value = v.y - parent_y
	
func rotate_peg(peg : Control, delta : float) -> void:
	var angle = peg.find_child("AngleText")
	if not angle: return
	angle.value += delta

func _input(event):
	if event is InputEventMouseButton:
		if (event.button_index == 1 || event.button_index == 2) and event.pressed and event.position.x < PLAYDATE_WIDTH:
			var yOff = $%LeftScroll.get_v_scroll()
			var v = Vector2(event.position.x, event.position.y + yOff)
			print("Mouse " , event.button_index , " Click/Unclick at: ", v)
			for peg_remove : Control in get_tree().get_nodes_in_group("pegs"):
				peg_remove.remove_theme_stylebox_override("panel")
			var peg : Control = find_peg(v, "static_pegs")
			if not peg: peg = find_peg(v, "elliptic_pegs")
			if not peg: peg = find_peg(v, "line_pegs")
			if peg:
				print("Found " , peg)
				peg.add_theme_stylebox_override("panel", load("res://selected_style_box_flat.tres"))
				$%RightScroll.set_v_scroll( peg.position.y )
				dragMode = event.button_index
				dragNode = peg
				dragPos = v
				return
			if not peg: peg = find_line_segment(v, "line_segments")
			if peg:
				var parentLineControl = peg.find_parent("LineControl")
				print("Found " , peg)
				parentLineControl.add_theme_stylebox_override("panel", load("res://selected_style_box_flat.tres"))
				$%RightScroll.set_v_scroll( parentLineControl.position.y )
				dragMode = event.button_index
				dragNode = peg
				dragPos = v
				return
			dragNode = null
		elif (event.button_index == 1 || event.button_index == 2) and not event.pressed:
			dragMode = 0
			#dragNode = null

	if event is InputEventMouseMotion and dragMode:
		var yOff = $%LeftScroll.get_v_scroll()
		var v = Vector2(event.position.x, event.position.y + yOff)
		print("Drag ", v)
		if dragMode == 1: 
			move_peg(dragNode, v)
		elif dragMode == 2:
			if abs(v.y - dragPos.y) > abs(v.x - dragPos.x):
				rotate_peg(dragNode, v.y - dragPos.y)
			else:
				rotate_peg(dragNode, v.x - dragPos.x)
			dragPos = v
		
