extends MenuButton

var selected_id : int = 0

@onready var rend = $/root/RootControl.find_child("Render")

func _ready():
	get_popup().id_pressed.connect(self.on_pressed)

func on_pressed(id: int):
	text = get_popup().get_item_text( id )
	selected_id = id
	rend.do_update()
