extends Node2D

const EDITOR_SCALE : float = 2.0

const BALL_RADIUS : float = 8.0 * EDITOR_SCALE
const BOX_WIDTH : float = 22.5 * EDITOR_SCALE
const BOX_HEIGHT : float = 12.5 * EDITOR_SCALE
const LINE_WIDTH : float = 2.0 * EDITOR_SCALE

const PLAYDATE_WIDTH : int = 400 * EDITOR_SCALE 

func sizeToScale(size : int) -> float:
	match size:
		0: return 1.0
		1: return 1.25
		2: return 1.5
		3: return 2.0
	return 1.0;
	
func renderPeg(circ : bool, x : int, y : int, a : float, size : int) -> void:
	if circ: renderCirc(x, y, size)
	else: renderRect(x, y, a, size)
	
func renderCirc(x : int, y : int, size : int) -> void:
	x *= EDITOR_SCALE
	y *= EDITOR_SCALE
	var s := sizeToScale(size)
	draw_circle(Vector2(x, y), BALL_RADIUS, Color.GRAY)
	draw_arc(Vector2(x, y), BALL_RADIUS, 0, 2*PI, 128, Color.BLACK, LINE_WIDTH)
	
func renderRect(x : int, y : int, a : float, size : int) -> void:
	x *= EDITOR_SCALE
	y *= EDITOR_SCALE
	var s := sizeToScale(size)
	a = (2*PI / 360.0) * a

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
		
	draw_polygon(points, [Color.GRAY])
	draw_polyline(points, Color.BLACK, 4)
	
func render_static_peg(staticPeg : Control):
	var circ : bool = staticPeg.find_child("CheckCirc").button_pressed
	var x : int = staticPeg.find_child("XText").value
	var y : int = staticPeg.find_child("YText").value
	var angle : int = staticPeg.find_child("AngleText").value
	var size : int = staticPeg.find_child("SizeText").shape_size
	renderPeg(circ, x, y, angle, size)

func do_update():
	print("Ping for redraw")
	queue_redraw()

func do_update_value(value):
	do_update()

func _draw():
	print("Redraw")
	
	for staticPeg in get_tree().get_nodes_in_group("static_pegs"):
		print("Render " + staticPeg.name)
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

