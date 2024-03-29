extends Node2D

const EDITOR_SCALE : int = 2

const BALL_RADIUS : float = 8.0 * EDITOR_SCALE
const BOX_WIDTH : float = 22.5 * EDITOR_SCALE
const BOX_HEIGHT : float = 12.5 * EDITOR_SCALE
const LINE_WIDTH : float = 2.0 * EDITOR_SCALE

const GRAB_CIRCLE_RADIUS : float = BALL_RADIUS/2

const PLAYDATE_WIDTH : int = 400 * EDITOR_SCALE

var dragMode : int = 0
var dragNode : Control = null
var dragY : float

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
	x *= EDITOR_SCALE
	y *= EDITOR_SCALE
	var s := sizeToScale(size)
	a = (2*PI / 360.0) * a
	if circ: renderCirc(x, y, s, type)
	else: renderRect(x, y, a, s, type)
	renderGrabCirc(x, y)
	
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
	
func renderGrabCirc(x : int, y : int) -> void:
	draw_circle(Vector2(x, y), GRAB_CIRCLE_RADIUS, Color.RED)
	draw_arc(Vector2(x, y), GRAB_CIRCLE_RADIUS, 0, 2*PI, 128, Color.DARK_RED, LINE_WIDTH)
	
func render_static_peg(staticPeg : Control):
	var circ : bool = staticPeg.find_child("CheckCirc").button_pressed
	var x : int = staticPeg.find_child("XText").value
	var y : int = staticPeg.find_child("YText").value
	var angle : int = staticPeg.find_child("AngleText").value
	var size : int = staticPeg.find_child("SizeText").shape_size
	var type : int = staticPeg.find_child("TypeText").shape_type
	renderPeg(circ, x, y, angle, size, type)

func do_update():
	print("Ping for redraw")
	queue_redraw()

func do_update_value(_value):
	do_update()

func _draw():
	var staticPegs = get_tree().get_nodes_in_group("static_pegs")
	print("Redraw ", len(staticPegs) , " static")
	for staticPeg in staticPegs:
		render_static_peg(staticPeg)
		
	draw_line(
		Vector2(0, $%HeightSlider.value*EDITOR_SCALE), Vector2(PLAYDATE_WIDTH, $%HeightSlider.value*EDITOR_SCALE),
	 	Color.RED, LINE_WIDTH*2)
	
	#var points = PackedVector2Array()
	#var colour = PackedColorArray()
	#points = [Vector2(100,100), Vector2(200,100), Vector2(200,200),Vector2(100,200)]
	#colour = [Color.GRAY]
#
	#draw_polygon(points, colour)
	#draw_polyline(points, Color.BLACK, 4)

func find_peg(v : Vector2, n : String) -> Control:
	for peg in get_tree().get_nodes_in_group(n):
		var v2 := Vector2()
		v2.x = peg.find_child("XText").value
		v2.y = peg.find_child("YText").value
		if v.distance_to(v2) < GRAB_CIRCLE_RADIUS:
			return peg
	return null
	
func move_peg(peg : Control, v : Vector2) -> void:
	peg.find_child("XText").value = v.x
	peg.find_child("YText").value = v.y
	
func rotate_peg(peg : Control, delta : float) -> void:
	peg.find_child("AngleText").value += delta

func _input(event):
	if event is InputEventMouseButton:
		if (event.button_index == 1 || event.button_index == 2) and event.pressed:
			var yOff = $%LeftScroll.get_v_scroll()
			var v = Vector2(event.position.x / EDITOR_SCALE, (event.position.y + yOff)/EDITOR_SCALE)
			print("Mouse " , event.button_index , " Click/Unclick at: ", v)
			var peg : Control = find_peg(v, "static_pegs")
			if not peg: peg = find_peg(v, "elliptic_pegs")
			if not peg: peg = find_peg(v, "path_pegs")
			if peg:
				print("Found " , peg)
				dragMode = event.button_index
				dragNode = peg
				dragY = v.y
		elif (event.button_index == 1 || event.button_index == 2) and not event.pressed:
			dragMode = 0
			dragNode = null

	if event is InputEventMouseMotion and dragMode:
		var yOff = $%LeftScroll.get_v_scroll()
		var v = Vector2(event.position.x / EDITOR_SCALE, (event.position.y + yOff)/EDITOR_SCALE)
		print("Drag ", v)
		if dragMode == 1: 
			move_peg(dragNode, v)
		elif dragMode == 2:
			rotate_peg(dragNode, v.y - dragY)
			dragY = v.y
		
