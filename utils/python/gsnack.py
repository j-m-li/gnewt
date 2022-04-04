# snack.py: maps C extension module _gsnack to proper python types in module
# snack.
# The first section is a very literal mapping.
# The second section contains convenience classes that amalgamate
# the literal classes and make them more object-oriented.

import _gsnack
import types
import string

snackArgs = {}
snackArgs['append'] = -1

FLAG_DISABLED = _gsnack.FLAG_DISABLED
FLAGS_SET = _gsnack.FLAGS_SET
FLAGS_RESET = _gsnack.FLAGS_RESET
FLAGS_TOGGLE = _gsnack.FLAGS_TOGGLE

class Widget:
    def setCallback(self, obj, data = None):
        if data:
            self.w.setCallback(obj, data)
        self.w.setCallback(obj)

class Button(Widget):

    def __init__(self, text):
	self.w = _gsnack.button(text)

class Checkbox(Widget):

    def value(self):
	return self.w.checkboxValue

    def selected(self):
	return self.w.checkboxValue != 0

    def setFlags (self, flag, sense):
        return self.w.checkboxSetFlags(flag, sense)

    def __init__(self, text, isOn = 0):
	self.w = _gsnack.checkbox(text, isOn)

class SingleRadioButton(Widget):

    def selected(self):
	return self.w.key == self.w.radioValue;
    
    def __init__(self, text, group, isOn = 0):
	if group:
	    self.w = _gsnack.radiobutton(text, group.w, isOn)
	else:
	    self.w = _gsnack.radiobutton(text, None, isOn)

class Listbox(Widget):

    def append(self, text, item):
	key = self.w.listboxAddItem(text)
	self.key2item[key] = item
	self.item2key[item] = key

    def insert(self, text, item, before):
	if (not before):
	    key = self.w.listboxInsertItem(text, 0)
	else:
	    key = self.w.listboxInsertItem(text, self.item2key[before])
	self.key2item[key] = item
	self.item2key[item] = key

    def delete(self, item):
	self.w.listboxDeleteItem(self.item2key[item])
	del self.key2item[self.item2key[item]]
	del self.item2key[item]

    def replace(self, text, item):
	key = self.w.listboxInsertItem(text, self.item2key[item])
	self.w.listboxDeleteItem(self.item2key[item])
	del self.key2item[self.item2key[item]]
	self.item2key[item] = key
	self.key2item[key] = item

    def current(self):
	return self.key2item[self.w.listboxGetCurrent()]

    def setCurrent(self, item):
	self.w.listboxSetCurrent(self.item2key[item])

    def __init__(self, height, scroll = 0, returnExit = 0, width = 0):
	self.w = _gsnack.listbox(height, scroll, returnExit)
	self.key2item = {}
	self.item2key = {}
	if (width):
	    self.w.listboxSetWidth(width)

class Textbox(Widget):

    def setText(self, text):
	self.w.textboxText(text)

    def __init__(self, width, height, text, scroll = 0, wrap = 0):
	self.w = _gsnack.textbox(width, height, text, scroll, wrap)

class TextboxReflowed(Textbox):

    def __init__(self, width, text, flexDown = 5, flexUp = 10, maxHeight = -1):
	(newtext, width, height) = reflow(text, width, flexDown, flexUp)
        if maxHeight != -1 and height > maxHeight:
            Textbox.__init__(self, width, maxHeight, newtext, 1)
        else:
            Textbox.__init__(self, width, height, newtext, 0)

class Label(Widget):

    def setText(self, text):
	self.w.labelText(text)

    def __init__(self, text):
	self.w = _gsnack.label(text)

class Scale(Widget):

    def set(self, amount):
	self.w.scaleSet(amount)

    def __init__(self, width, total):
	self.w = _gsnack.scale(width, total)

class Entry(Widget):

    def value(self):
	return self.w.entryValue

    def set(self, text):
	return self.w.entrySetValue(text)

    def setFlags (self, flag, sense):
        return self.w.entrySetFlags(flag, sense)

    def __init__(self, width, text = "", hidden = 0, scroll = 1, 
		 returnExit = 0):
	self.w = _gsnack.entry(width, text, hidden, scroll, returnExit)


# Form uses hotkeys
hotkeys = { "F1" : _gsnack.KEY_F1, "F2" : _gsnack.KEY_F2, "F3" : _gsnack.KEY_F3, 
            "F4" : _gsnack.KEY_F4, "F5" : _gsnack.KEY_F5, "F6" : _gsnack.KEY_F6, 
            "F7" : _gsnack.KEY_F7, "F8" : _gsnack.KEY_F8, "F9" : _gsnack.KEY_F9, 
            "F10" : _gsnack.KEY_F10, "F11" : _gsnack.KEY_F11, 
            "F12" : _gsnack.KEY_F12 }

for n in hotkeys.keys():
    hotkeys[hotkeys[n]] = n

class Form:

    def addHotKey(self, keyname):
	self.w.addhotkey(hotkeys[keyname])

    def add(self, widget):
	if widget.__dict__.has_key('hotkeys'):
	    for key in widget.hotkeys.keys():
		self.addHotKey(key)

	if widget.__dict__.has_key('gridmembers'):
	    for w in widget.gridmembers:
		self.add(w)
	elif widget.__dict__.has_key('w'):
	    self.trans[widget.w.key] = widget
	    return self.w.add(widget.w)
	return None

    def run(self):
	(what, which) = self.w.run()
	if (what == _gsnack.FORM_EXIT_WIDGET):
	    return self.trans[which]

	return hotkeys[which]

    def draw(self):
	self.w.draw()
	return None

    def __init__(self):
	self.trans = {}
	self.w = _gsnack.form()

    def setCurrent (self, co):
        self.w.setcurrent (co.w)

class Grid:

    def place(self, x, y):
	return self.g.place(x, y)

    def setField(self, what, col, row, padding = (0, 0, 0, 0),
		 anchorLeft = 0, anchorTop = 0, anchorRight = 0,
		 anchorBottom = 0, growx = 0, growy = 0):
	self.gridmembers.append(what)
	anchorFlags = 0
	if (anchorLeft):
	    anchorFlags = _gsnack.ANCHOR_LEFT
	elif (anchorRight):
	    anchorFlags = _gsnack.ANCHOR_RIGHT

	if (anchorTop):
	    anchorFlags = anchorFlags | _gsnack.ANCHOR_TOP
	elif (anchorBottom):
	    anchorFlags = anchorFlags | _gsnack.ANCHOR_BOTTOM

	gridFlags = 0
	if (growx):
	    gridFlags = _gsnack.GRID_GROWX
	if (growy):
	    gridFlags = gridFlags | _gsnack.GRID_GROWY

	if (what.__dict__.has_key('g')):
	    return self.g.setfield(col, row, what.g, padding, anchorFlags,
				   gridFlags)
	else:
	    return self.g.setfield(col, row, what.w, padding, anchorFlags)

    def __init__(self, *args):
	self.g = apply(_gsnack.grid, args)
	self.gridmembers = []

class SnackScreen:

    def __init__(self):
	_gsnack.init()
	(self.width, self.height) = _gsnack.size()
	self.pushHelpLine(None)

    def finish(self):
	return _gsnack.finish()

    def resume(self):
	_gsnack.resume()

    def suspend(self):
	_gsnack.suspend()

    def suspendCallback(self, cb, data = None):
        if data:
            return _gsnack.suspendcallback(cb, data)
        return _gsnack.suspendcallback(cb)

    def openWindow(self, left, top, width, height, title):
	return _gsnack.openwindow(left, top, width, height, title)

    def pushHelpLine(self, text):
	if (not text):
	    return _gsnack.pushhelpline("*default*")
	else:
	    return _gsnack.pushhelpline(text)

    def popHelpLine(self):
	return _gsnack.pophelpline()

    def drawRootText(self, left, top, text):
	return _gsnack.drawroottext(left, top, text)

    def centeredWindow(self, width, height, title):
	return _gsnack.centeredwindow(width, height, title)

    def gridWrappedWindow(self, grid, title):
	return _gsnack.gridwrappedwindow(grid.g, title)

    def popWindow(self):
	return _gsnack.popwindow()

    def refresh(self):
	return _gsnack.refresh()

# returns a tuple of the wrapped text, the actual width, and the actual height
def reflow(text, width, flexDown = 5, flexUp = 5):
    return _gsnack.reflow(text, width, flexDown, flexUp)

# combo widgets

class RadioGroup(Widget):

    def __init__(self):
	self.prev = None
	self.buttonlist = []

    def add(self, title, value, default = None):
	if not self.prev and default == None:
	    # If the first element is not explicitly set to
	    # not be the default, make it be the default
	    default = 1
	b = SingleRadioButton(title, self.prev, default)
	self.prev = b
	self.buttonlist.append((b, value))
	return b

    def getSelection(self):
	for (b, value) in self.buttonlist:
	    if b.selected(): return value
	return None


class RadioBar(Grid):

    def __init__(self, screen, buttonlist):
	self.list = []
	self.item = 0
	self.group = RadioGroup()
	Grid.__init__(self, 1, len(buttonlist))
	for (title, value, default) in buttonlist:
	    b = self.group.add(title, value, default)
	    self.list.append(b, value)
	    self.setField(b, 0, self.item, anchorLeft = 1)
	    self.item = self.item + 1

    def getSelection(self):
	return self.group.getSelection()
	

# you normally want to pack a ButtonBar with growx = 1

class ButtonBar(Grid):

    def __init__(self, screen, buttonlist):
	self.list = []
	self.hotkeys = {}
	self.item = 0
	Grid.__init__(self, len(buttonlist), 1)
	for blist in buttonlist:
	    if (type(blist) == types.StringType):
		title = blist
		value = string.lower(blist)
	    elif len(blist) == 2:
		(title, value) = blist
	    else:
		(title, value, hotkey) = blist
		self.hotkeys[hotkey] = value

	    b = Button(title)
	    self.list.append(b, value)
	    self.setField(b, self.item, 0, (1, 0, 1, 0))
	    self.item = self.item + 1

    def buttonPressed(self, result):
	"""Takes the widget returned by Form.run and looks to see
	if it was one of the widgets in the ButtonBar."""

	if self.hotkeys.has_key(result):
	    return self.hotkeys[result]

	for (button, value) in self.list:
	    if result == button:
		return value
	return None


class GridForm(Grid):

    def __init__(self, screen, title, *args):
	self.screen = screen
	self.title = title
	self.form = Form()
	self.childList = []
	self.form_created = 0
	args = list(args)
	args[:0] = [self]
	apply(Grid.__init__, tuple(args))

    def add(self, widget, col, row, padding = (0, 0, 0, 0),
            anchorLeft = 0, anchorTop = 0, anchorRight = 0,
            anchorBottom = 0, growx = 0, growy = 0):
	self.setField(widget, col, row, padding, anchorLeft,
		      anchorTop, anchorRight, anchorBottom,
		      growx, growy);
	self.childList.append(widget)

    def runOnce(self):
	result = self.run()
	self.screen.popWindow()
	return result

    def addHotKey(self, keyname):
	self.form.addHotKey(keyname)

    def create(self):
	if not self.form_created:
	    self.place(1,1)
	    for child in self.childList:
		self.form.add(child)
	    self.screen.gridWrappedWindow(self, self.title)
	    self.form_created = 1

    def run(self):
	self.create()
	return self.form.run()

    def draw(self):
	self.create()
	return self.form.draw()
	
    def runPopup(self):
	self.create()
	self.screen.gridWrappedWindow(self, self.title)
	result = self.form.run()
	self.screen.popWindow()
	return result

    def setCurrent (self, co):
        self.form.setCurrent (co)

class CheckboxTree(Widget):
    def append(self, text, item = None, selected = 0):
    	self.addItem(text, (snackArgs['append'], ), item, selected)

    def addItem(self, text, path, item = None, selected = 0):
    	if (not item):
	    item = text
	key = self.w.checkboxtreeAddItem(text, path, selected)
	self.key2item[key] = item
	self.item2key[item] = key

    def __init__(self, height, scroll = 0):
	self.w = _gsnack.checkboxtree(height, scroll)
	self.key2item = {}
	self.item2key = {}

    def getSelection(self):
        selection = []
        list = self.w.checkboxtreeGetSelection()
        for key in list:
            selection.append(self.key2item[key])
	return selection


def ListboxChoiceWindow(screen, title, text, items, 
			buttons = ('Ok', 'Cancel'), 
			width = 40, scroll = 0, height = -1, default = None):
    if (height == -1): height = len(items)

    bb = ButtonBar(screen, buttons)
    t = TextboxReflowed(width, text)
    l = Listbox(height, scroll = scroll, returnExit = 1)
    count = 0
    for item in items:
	if (type(item) == types.TupleType):
	    (text, key) = item
	else:
	    text = item
	    key = count

	if (default == count):
	    default = key
	elif (default == item):
	    default = key

	l.append(text, key)
	count = count + 1

    if (default != None):
	l.setCurrent (default)

    g = GridForm(screen, title, 1, 3)
    g.add(t, 0, 0)
    g.add(l, 0, 1, padding = (0, 1, 0, 1))
    g.add(bb, 0, 2, growx = 1)

    rc = g.runOnce()

    return (bb.buttonPressed(rc), l.current())

def ButtonChoiceWindow(screen, title, text, 
		       buttons = [ 'Ok', 'Cancel' ], 
		       width = 40):
    bb = ButtonBar(screen, buttons)
    t = TextboxReflowed(width, text, maxHeight = screen.height - 12)

    g = GridForm(screen, title, 1, 2)
    g.add(t, 0, 0, padding = (0, 0, 0, 1))
    g.add(bb, 0, 1, growx = 1)
    return bb.buttonPressed(g.runOnce())

def EntryWindow(screen, title, text, prompts, allowCancel = 1, width = 40,
		entryWidth = 20, buttons = [ 'Ok', 'Cancel' ]):
    bb = ButtonBar(screen, buttons);
    t = TextboxReflowed(width, text)

    count = 0
    for n in prompts:
	count = count + 1

    sg = Grid(2, count)

    count = 0
    entryList = []
    for n in prompts:
	if (type(n) == types.TupleType):
	    (n, e) = n
	else:
	    e = Entry(entryWidth)

	sg.setField(Label(n), 0, count, padding = (0, 0, 1, 0), anchorLeft = 1)
	sg.setField(e, 1, count, anchorLeft = 1)
	count = count + 1
	entryList.append(e)

    g = GridForm(screen, title, 1, 3)

    g.add(t, 0, 0, padding = (0, 0, 0, 1))
    g.add(sg, 0, 1, padding = (0, 0, 0, 1))
    g.add(bb, 0, 2, growx = 1)

    result = g.runOnce()

    entryValues = []
    count = 0
    for n in prompts:
	entryValues.append(entryList[count].value())
	count = count + 1

    return (bb.buttonPressed(result), tuple(entryValues))
