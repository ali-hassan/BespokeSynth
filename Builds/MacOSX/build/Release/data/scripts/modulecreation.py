import module

if 'osc' in globals():
   osc.delete()
osc = module.create("oscillator", 100, 550)
osc.set_target(module.get("gain"))  
module.get("notestream").set_target(osc)

def on_pulse():
   step = bespoke.get_step(8) % 8
   osc.set_position(150 * step, 550)
   this.play_note_pan(bespoke.get_root() + bespoke.get_scale()[step%7] + 36 + step / 7 * 12, 127, 1.0/8, step / 4.0 - 1)
    