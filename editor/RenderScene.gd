extends Node2D

const BALL_RADIUS : float = 8.0
const BOX_WIDTH : float = 22.5
const BOX_HEIGHT : float = 12.5
const HEX_WIDTH : float = 22.5/2
const LINE_WIDTH : float = 2.0

const PLAYDATE_WIDTH : int = 400

const GRAB_CIRCLE_RADIUS : float = BALL_RADIUS/2

var t : float = 0

func sizeToScale(size : int) -> float:
	match size:
		0: return 1.25
		1: return 1.5
		2: return 1.75
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
	if type == 2: renderRegularPoly(6, x, y, a, s, type)
	elif circ: renderCirc(x, y, s, type)
	else: renderRect(x, y, a, s, type)
	
func renderCirc(x : int, y : int, s : float, type : int) -> void:
	draw_circle(Vector2(x, y), BALL_RADIUS*s, fillColor(type))
	draw_arc(Vector2(x, y), BALL_RADIUS*s, 0, 2*PI, 128, lineColor(type), LINE_WIDTH)
	

func renderRegularPoly(corners : int, x : int, y : int, a : float, s : float, type : int) -> void:
	var points : PackedVector2Array
	var angleAdvance = (2*PI) / corners
	var w2 = HEX_WIDTH * s
	var h2 = HEX_WIDTH * s
	for p in range(corners):
		var angle = (angleAdvance * p) + a
		var vp : Vector2
		vp.x = x + (w2 * cos(angle))
		vp.y = y + (h2 * sin(angle))
		points.append(vp)
	points.append( points[0] )
	draw_polygon(points, [fillColor(type)])
	draw_polyline(points, lineColor(type), LINE_WIDTH)

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
	draw_polyline(points, lineColor(type), LINE_WIDTH)

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
	render_grab_shape(x, y, staticPeg == %InputParser.dragNode)

func render_elliptic_path(ellipticPath : Control):
	var n_pegs : int = ellipticPath.find_child("PathSlider").value
	var circ : bool = ellipticPath.find_child("CheckCirc").button_pressed
	var size : int = ellipticPath.find_child("SizeText").selected
	var ease : int = ellipticPath.find_child("EaseText").selected
	var x : int = ellipticPath.find_child("XText").value
	var y : int = ellipticPath.find_child("YText").value
	var arc_rad : float = ellipticPath.find_child("ArcText").value * (2*PI / 360.0)
	var speed : float = ellipticPath.find_child("SpeedText").value
	var a : int = ellipticPath.find_child("AText").value
	var b : int = ellipticPath.find_child("BText").value
	var angle_rad : float = ellipticPath.find_child("AngleText").value * (2*PI / 360.0) 
	var use_arc : int = ellipticPath.find_child("ArcAngleCheckbox").button_pressed
	
	for i in range(0, n_pegs):
		var peg_container : Control = ellipticPath.find_child("PegContainer"+str(i+1), true, false)
		var custom_shape = peg_container.find_child("ShapeButton").selected
		var custom_size = peg_container.find_child("SizeButton").selected
		var custom_type = peg_container.find_child("TypeButton").selected
		var shape_peg = circ
		var size_peg = size
		if custom_shape:
			shape_peg = 1 if custom_shape == 0 else 0
		if custom_size:
			size_peg = custom_size-1
		var arc_peg : float = (arc_rad / n_pegs) * i
		var x_peg : float = x + (a * cos((t * speed) + arc_peg))
		var y_peg : float = y + (b * sin((t * speed) + arc_peg))
		var draw_angle : float
		if use_arc:
			draw_angle = (t * speed) + arc_peg + angle_rad
		else:
			draw_angle = angle_rad
		renderPeg(shape_peg, x_peg, y_peg, draw_angle, size_peg, custom_type)
	render_grab_shape(x, y, ellipticPath == %InputParser.dragNode)
	
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
	var arc_rad : float = linePath.find_child("ArcText").value * (2*PI / 360.0) 
	var speed : float = linePath.find_child("SpeedText").value
	var angle_rad : float = linePath.find_child("AngleText").value * (2*PI / 360.0)
	var use_arc : int = linePath.find_child("ArcAngleCheckbox").button_pressed
	
	var line_path_array : Array
	line_path_array.push_back(Vector2(x,y))
	for i in range(0, n_lines):
		var line_container : Control = linePath.find_child("LineContainer"+str(i+1), true, false)
		my_nodes.push_back(line_container)
		var x_off = line_container.find_child("XOffText").value
		var y_off = line_container.find_child("YOffText").value
		line_path_array.push_back(Vector2(x+x_off, y+y_off))
		if (y+y_off) > %HeightSlider.value:
			line_container.find_child("YOffText").value = %HeightSlider.value - y
	line_path_array.push_back(Vector2(x,y))
	
	var line_path_lengths : Array
	var tot_path_length : float = 0
	for i in range(0, len(line_path_array)-1):
		line_path_lengths.push_back( line_path_array[i].distance_to( line_path_array[i+1] ) )
		tot_path_length += line_path_lengths.back()
		
	for i in range(0, n_pegs):
		var peg_container : Control = linePath.find_child("PegContainer"+str(i+1), true, false)
		var custom_shape = peg_container.find_child("ShapeButton").selected
		var custom_size = peg_container.find_child("SizeButton").selected
		var custom_type = peg_container.find_child("TypeButton").selected
		var shape_peg = circ
		var size_peg = size
		if custom_shape:
			shape_peg = 1 if custom_shape == 0 else 0
		if custom_size:
			size_peg = custom_size-1
		var arc_peg : float = (arc_rad / n_pegs) * i
		
		var draw_angle : float
		if use_arc:
			draw_angle = (t * speed) + arc_peg + angle_rad
		else:
			draw_angle = angle_rad
		
		var path_step : int = 0
		var step_len_frac : float = 0;
		var tot_len_frac : float = ((t * speed) + arc_peg) / (2*PI);
		tot_len_frac = tot_len_frac - floor(tot_len_frac)
		
		while true:
			step_len_frac = line_path_lengths[path_step] / tot_path_length
			if tot_len_frac < step_len_frac:
				break
			else:
				tot_len_frac -= step_len_frac
				path_step += 1
		# totLenFrac is now 0-X, where X is this steps fraction of the total path length: stepLenFrac.
		# we want to scale this back into the range 0-1 and apply easing
		#const float lenFrac = getEasing(p->m_easing, totLenFrac / stepLenFrac);
		var len_frac : float = tot_len_frac / step_len_frac
		# and we scale the difference between the points by this value
		var dx : float = (line_path_array[path_step+1].x - line_path_array[path_step].x) * len_frac
		var dy : float = (line_path_array[path_step+1].y - line_path_array[path_step].y) * len_frac
		var x_peg = line_path_array[path_step].x + dx
		var y_peg = line_path_array[path_step].y + dy
		renderPeg(shape_peg, x_peg, y_peg, draw_angle, size_peg, custom_type)
		
	render_line_path_grab_shapes(line_path_array, %InputParser.dragNode in my_nodes)
	render_grab_shape(x, y, %InputParser.dragNode in my_nodes)
	
#
func _process(delta):
	t += delta
	#if t >= 2*PI: t -= 2*PI
	queue_redraw()
	%BackgroundTexRect.position.y += delta * 15.0
	if %BackgroundTexRect.position.y >= 0:
		%BackgroundTexRect.position.y -= 60
	
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
		Vector2(0, %HeightSlider.value), Vector2(PLAYDATE_WIDTH, %HeightSlider.value),
	 	Color.RED, LINE_WIDTH*2)
		
	for i in range(4):
		draw_line(
			Vector2(0, 240*i), Vector2(PLAYDATE_WIDTH, 240*i),
		 	Color.WEB_GRAY, LINE_WIDTH)
