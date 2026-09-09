"""Create crab animation frames by posing a crab config's limbs for each frame."""

from __future__ import annotations

import argparse
import copy
import json
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, simpledialog, ttk
from typing import Any

from PIL import Image, ImageOps, ImageTk

from make_crab_config import (
	CRAB_CONFIG_DIR,
	crab_config_names,
	leg_config_key,
	load_mapped_config,
	load_mapped_image,
	resolve_body_stem,
	resolve_claw_stem,
	resolve_leg_stem,
)

PROJECT_DIR = Path(__file__).resolve().parent
ANIMATION_DIR = PROJECT_DIR / "animation_configs"
MAX_ZOOM = 128

LEG_ATTACHMENTS = (
	"left_front_leg",
	"left_middle_leg",
	"left_rear_leg",
	"right_front_leg",
	"right_middle_leg",
	"right_rear_leg",
)
LEFT_LEG_ATTACHMENTS = (
	"left_front_leg",
	"left_middle_leg",
	"left_rear_leg",
)
RIGHT_LEG_ATTACHMENTS = (
	"right_front_leg",
	"right_middle_leg",
	"right_rear_leg",
)
CLAW_ATTACHMENTS = ("left_claw", "right_claw")
ATTACHMENT_NAMES = LEG_ATTACHMENTS + CLAW_ATTACHMENTS


def active_attachments_for_direction(direction: str) -> tuple[str, ...]:
	if direction == "right":
		return RIGHT_LEG_ATTACHMENTS + CLAW_ATTACHMENTS
	elif direction == "left":
		return LEFT_LEG_ATTACHMENTS + CLAW_ATTACHMENTS
	else:
		return ATTACHMENT_NAMES


def stem_for_attachment(crab_config: dict[str, str], name: str, direction: str) -> str:
	if name in CLAW_ATTACHMENTS:
		return resolve_claw_stem(crab_config[name], direction)
	side, position, _ = name.split("_")
	return resolve_leg_stem(crab_config[leg_config_key(side, position)], direction, position)


def load_crab_config(name: str) -> dict[str, Any]:
	with (CRAB_CONFIG_DIR / f"{name}.json").open(encoding="utf-8") as config_file:
		return json.load(config_file)


class AnimationMapper:
	"""Edit a sequence of body-local limb attachment points."""

	def __init__(self, root: tk.Tk, animation_name: str, crab_config_name: str) -> None:
		self.root = root
		self.animation_name = animation_name
		self.crab_config_name = tk.StringVar(value=crab_config_name)
		self.crab_config: dict[str, str] = {}
		self.direction = tk.StringVar(value="up")
		self.part_widgets: dict[str, ttk.Frame] = {}
		self.body_config: dict[str, Any] = {}
		self.part_configs: dict[str, dict[str, Any]] = {}
		self.body = Image.new("RGBA", (1, 1))
		self.part_images: dict[str, Image.Image] = {}
		self.zoom = 4
		self.frames: list[dict[str, Any]] = []
		self.current_frame_index = 0
		self.current_points: dict[str, dict[str, int]] = {}
		self.selected_point = tk.StringVar(value=ATTACHMENT_NAMES[0])
		self.part_rotations = {name: tk.DoubleVar(value=0.0) for name in ATTACHMENT_NAMES}
		self.preview_image: ImageTk.PhotoImage | None = None
		self.body_origin = (0, 0)
		self.canvas_size = 1
		self.sidebar_canvases: dict[str, tk.Canvas] = {}
		self.sidebar_widths = {"left": 225, "right": 225}
		self.resize_start_x = 0
		self.resize_start_width = 0
		self.is_playing = False
		self.play_timer: str | None = None
		self.spf_var = tk.DoubleVar(value=1.0)

		root.title(f"Crab Animation Mapper: {animation_name}")
		root.resizable(True, True)
		root.minsize(850, 650)

		self.title_label = ttk.Label(root, font=("TkDefaultFont", 14, "bold"))
		self.title_label.pack(anchor="w", padx=12, pady=(12, 4))
		self.status_label = ttk.Label(root)
		self.status_label.pack(anchor="w", padx=12, pady=(0, 10))

		content = ttk.Frame(root, padding=(12, 0, 12, 12))
		content.pack(fill="both", expand=True)
		content.columnconfigure(2, weight=1)
		content.rowconfigure(0, weight=1)

		point_frame = self.create_scrollable_sidebar(content, 0, "left", "Parts and attachment points")
		self.create_resize_handle(content, 1, "left")
		ttk.Label(point_frame, text="Facing direction").pack(anchor="w")
		direction_selector = ttk.Combobox(point_frame, textvariable=self.direction, values=("up", "down", "right", "left"), state="readonly", width=18)
		direction_selector.pack(anchor="w", pady=(0, 8))
		direction_selector.bind("<<ComboboxSelected>>", self.change_direction)
		for name in ATTACHMENT_NAMES:
			self.add_part_widget(point_frame, name, name.replace("_", " ").title())

		self.apply_direction_defaults()

		self.canvas = tk.Canvas(content, background="#303030", highlightthickness=0)
		self.canvas.grid(row=0, column=2, sticky="nsew")
		self.canvas.bind("<Button-1>", self.move_selected_point)
		self.canvas.bind("<B1-Motion>", self.move_selected_point)
		self.canvas.bind("<Button-3>", self.start_pan)
		self.canvas.bind("<B3-Motion>", self.pan)
		self.canvas.bind("<MouseWheel>", self.change_zoom)
		self.canvas.bind("<Button-4>", self.change_zoom)
		self.canvas.bind("<Button-5>", self.change_zoom)

		self.create_resize_handle(content, 3, "right")
		controls = self.create_scrollable_sidebar(content, 4, "right", "Animation controls")
		ttk.Label(controls, text="Crab config").pack(anchor="w")
		crab_config_selector = ttk.Combobox(controls, textvariable=self.crab_config_name, values=crab_config_names(), state="readonly", width=18)
		crab_config_selector.pack(anchor="w", fill="x", pady=(0, 8))
		crab_config_selector.bind("<<ComboboxSelected>>", self.change_crab_config)
		ttk.Button(controls, text="Save Animation", command=self.save_animation).pack(fill="x", pady=(0, 8))
		ttk.Button(controls, text="Save Frame", command=self.save_frame).pack(fill="x", pady=(0, 8))
		self.next_button = ttk.Button(controls, text="Next Frame", command=self.next_frame)
		self.next_button.pack(fill="x", pady=(0, 8))
		self.previous_button = ttk.Button(controls, text="Previous Frame", command=self.previous_frame)
		self.previous_button.pack(fill="x", pady=(0, 8))
		self.frame_indicator = ttk.Label(controls)
		self.frame_indicator.pack(anchor="w", pady=(0, 8))

		playback_frame = ttk.LabelFrame(controls, text="Playback", padding=6)
		playback_frame.pack(fill="x", pady=(0, 8))

		self.play_button = ttk.Button(playback_frame, text="Play", command=self.toggle_play)
		self.play_button.pack(fill="x", pady=(0, 6))

		ttk.Label(playback_frame, text="Seconds / frame:").pack(anchor="w")
		spf_spinbox = ttk.Spinbox(playback_frame, from_=0.05, to=10.0, increment=0.05, textvariable=self.spf_var, width=10)
		spf_spinbox.pack(anchor="w", fill="x")

		self.redraw_canvas()

	def create_scrollable_sidebar(self, parent: ttk.Frame, column: int, name: str, title: str) -> ttk.LabelFrame:
		sidebar = ttk.Frame(parent)
		sidebar.grid(row=0, column=column, sticky="ns")
		sidebar.rowconfigure(0, weight=1)
		sidebar.columnconfigure(0, weight=1)
		canvas = tk.Canvas(sidebar, highlightthickness=0, width=self.sidebar_widths[name])
		self.sidebar_canvases[name] = canvas
		scrollbar = ttk.Scrollbar(sidebar, orient="vertical", command=canvas.yview)
		canvas.configure(yscrollcommand=scrollbar.set)
		canvas.grid(row=0, column=0, sticky="ns")
		scrollbar.grid(row=0, column=1, sticky="ns")
		content = ttk.LabelFrame(canvas, text=title, padding=8)
		window = canvas.create_window((0, 0), anchor="nw", window=content)
		content.bind("<Configure>", lambda event: canvas.configure(scrollregion=canvas.bbox("all")))
		canvas.bind("<Configure>", lambda event: canvas.itemconfigure(window, width=event.width))
		canvas.bind("<Enter>", lambda event: self.enable_sidebar_scroll(canvas))
		canvas.bind("<Leave>", lambda event: self.disable_sidebar_scroll(canvas))
		return content

	def create_resize_handle(self, parent: ttk.Frame, column: int, name: str) -> None:
		handle = tk.Frame(parent, width=6, cursor="sb_h_double_arrow")
		handle.grid(row=0, column=column, sticky="ns")
		handle.bind("<Button-1>", lambda event, sidebar_name=name: self.start_sidebar_resize(event, sidebar_name))
		handle.bind("<B1-Motion>", lambda event, sidebar_name=name: self.resize_sidebar(event, sidebar_name))

	def start_sidebar_resize(self, event: tk.Event[tk.Misc], name: str) -> None:
		self.resize_start_x = event.x_root
		self.resize_start_width = self.sidebar_widths[name]

	def resize_sidebar(self, event: tk.Event[tk.Misc], name: str) -> None:
		delta = event.x_root - self.resize_start_x
		if name == "right":
			delta = -delta
		width = max(150, min(480, self.resize_start_width + delta))
		self.sidebar_widths[name] = width
		self.sidebar_canvases[name].configure(width=width)

	def enable_sidebar_scroll(self, canvas: tk.Canvas) -> None:
		canvas.bind_all("<MouseWheel>", lambda event: canvas.yview_scroll(-event.delta // 120, "units"))
		canvas.bind_all("<Button-4>", lambda event: canvas.yview_scroll(-1, "units"))
		canvas.bind_all("<Button-5>", lambda event: canvas.yview_scroll(1, "units"))

	def disable_sidebar_scroll(self, canvas: tk.Canvas) -> None:
		canvas.unbind_all("<MouseWheel>")
		canvas.unbind_all("<Button-4>")
		canvas.unbind_all("<Button-5>")

	def add_part_widget(self, parent: ttk.LabelFrame, name: str, label: str) -> None:
		frame = ttk.LabelFrame(parent, text=label, padding=6)
		self.part_widgets[name] = frame
		ttk.Label(frame, text="Rotation").pack(anchor="w")
		rotation_slider = ttk.Scale(
			frame,
			from_=-180,
			to=180,
			variable=self.part_rotations[name],
			command=lambda value, part_name=name: self.change_rotation(part_name, value),
		)
		rotation_slider.pack(anchor="w", fill="x", pady=(0, 2))
		ttk.Radiobutton(
			frame,
			text="Move attachment",
			value=name,
			variable=self.selected_point,
		).pack(anchor="w", pady=(0, 2))

	def update_part_widgets(self) -> None:
		active = active_attachments_for_direction(self.direction.get())
		for name, frame in self.part_widgets.items():
			if name in active:
				frame.pack(anchor="w", fill="x", pady=(0, 6))
			else:
				frame.pack_forget()
		if self.selected_point.get() not in active:
			self.selected_point.set(active[0])

	def apply_direction_defaults(self) -> None:
		self.update_part_widgets()
		self.load_crab_assets(reset_points=True)

	def load_crab_assets(self, reset_points: bool) -> None:
		direction = self.direction.get()
		active = active_attachments_for_direction(direction)
		try:
			self.crab_config = load_crab_config(self.crab_config_name.get())
			body_stem = resolve_body_stem(self.crab_config["body"], direction)
			self.body_config = load_mapped_config(body_stem)
			self.body = load_mapped_image(body_stem)
			if direction == "left":
				self.body = ImageOps.mirror(self.body)
			stems = {name: stem_for_attachment(self.crab_config, name, direction) for name in active}
			self.part_configs = {name: load_mapped_config(stem) for name, stem in stems.items()}
			self.part_images = {name: load_mapped_image(stem) for name, stem in stems.items()}
		except (FileNotFoundError, KeyError, ValueError) as error:
			messagebox.showerror("Crab config error", f"Could not resolve assets from the crab config: {error}")
			return

		max_part_size = max(max(image.width, image.height) for image in self.part_images.values())
		self.body_origin = (max_part_size, max_part_size)
		self.canvas_size = self.body.width + 2 * max_part_size
		if reset_points:
			body_points = copy.deepcopy(self.body_config["attachment_points"])
			if direction == "left":
				body_width = self.body.width
				for pt_name, pt in list(body_points.items()):
					pt["x"] = body_width - 1 - pt["x"]
			self.set_current_points(body_points)

	def change_direction(self, event: tk.Event[tk.Misc]) -> None:
		self.stop_play()
		self.frames = []
		self.current_frame_index = 0
		self.apply_direction_defaults()
		self.redraw_canvas()

	def change_crab_config(self, event: tk.Event[tk.Misc]) -> None:
		self.load_crab_assets(reset_points=False)
		self.redraw_canvas()

	def change_rotation(self, name: str, value: str) -> None:
		self.current_points[name]["rotation"] = float(value)
		self.redraw_canvas()

	def set_current_points(self, points: dict[str, dict[str, Any]]) -> None:
		direction = self.direction.get()
		active = active_attachments_for_direction(direction)
		self.current_points = {}
		for name in active:
			if name in points:
				self.current_points[name] = copy.deepcopy(points[name])
			elif name in self.body_config.get("attachment_points", {}):
				self.current_points[name] = copy.deepcopy(self.body_config["attachment_points"][name])
			else:
				self.current_points[name] = {"x": self.body.width // 2, "y": self.body.height // 2}
			rotation = float(self.current_points[name].get("rotation", 0.0))
			self.current_points[name]["rotation"] = rotation
			self.part_rotations[name].set(rotation)

	def part_with_anchor(self, image: Image.Image, config: dict[str, Any], mirror: bool, rotation: float) -> tuple[Image.Image, tuple[int, int]]:
		anchor_x = config["attachment_point_x"]
		anchor_y = config["attachment_point_y"]
		if mirror:
			image = ImageOps.mirror(image)
			anchor_x = image.width - 1 - anchor_x
		return self.rotate_around_anchor(image, (anchor_x, anchor_y), rotation)

	def rotate_around_anchor(self, image: Image.Image, anchor: tuple[int, int], rotation: float) -> tuple[Image.Image, tuple[int, int]]:
		anchor_x, anchor_y = anchor
		radius = max(anchor_x, anchor_y, image.width - 1 - anchor_x, image.height - 1 - anchor_y)
		size = 2 * radius + 1
		padded_part = Image.new("RGBA", (size, size))
		padded_part.alpha_composite(image, (radius - anchor_x, radius - anchor_y))
		rotated_part = padded_part.rotate(rotation, resample=Image.Resampling.NEAREST, fillcolor=(0, 0, 0, 0))
		return rotated_part, (radius, radius)

	def should_mirror_leg(self, name: str) -> bool:
		return name.startswith("left_") if self.direction.get() == "up" else name.startswith("right_")

	def composite_preview(self) -> Image.Image:
		direction = self.direction.get()
		canvas = Image.new("RGBA", (self.canvas_size, self.canvas_size))

		if direction in ("up", "down"):
			for name in LEG_ATTACHMENTS:
				part, anchor = self.part_with_anchor(self.part_images[name], self.part_configs[name], self.should_mirror_leg(name), self.current_points[name].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points[name])
			for name in CLAW_ATTACHMENTS:
				part, anchor = self.part_with_anchor(self.part_images[name], self.part_configs[name], name.startswith("left_"), self.current_points[name].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points[name])
			canvas.alpha_composite(self.body, self.body_origin)

		elif direction == "right":
			if "left_claw" in self.part_images:
				part, anchor = self.part_with_anchor(self.part_images["left_claw"], self.part_configs["left_claw"], False, self.current_points["left_claw"].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points["left_claw"])

			canvas.alpha_composite(self.body, self.body_origin)

			for name in RIGHT_LEG_ATTACHMENTS:
				if name in self.part_images:
					part, anchor = self.part_with_anchor(self.part_images[name], self.part_configs[name], False, self.current_points[name].get("rotation", 0.0))
					self.paste_part(canvas, part, anchor, self.current_points[name])

			if "right_claw" in self.part_images:
				part, anchor = self.part_with_anchor(self.part_images["right_claw"], self.part_configs["right_claw"], False, self.current_points["right_claw"].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points["right_claw"])

		elif direction == "left":
			if "right_claw" in self.part_images:
				part, anchor = self.part_with_anchor(self.part_images["right_claw"], self.part_configs["right_claw"], True, self.current_points["right_claw"].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points["right_claw"])

			canvas.alpha_composite(self.body, self.body_origin)

			for name in LEFT_LEG_ATTACHMENTS:
				if name in self.part_images:
					part, anchor = self.part_with_anchor(self.part_images[name], self.part_configs[name], True, self.current_points[name].get("rotation", 0.0))
					self.paste_part(canvas, part, anchor, self.current_points[name])

			if "left_claw" in self.part_images:
				part, anchor = self.part_with_anchor(self.part_images["left_claw"], self.part_configs["left_claw"], True, self.current_points["left_claw"].get("rotation", 0.0))
				self.paste_part(canvas, part, anchor, self.current_points["left_claw"])

		return canvas

	def paste_part(self, canvas: Image.Image, part: Image.Image, part_anchor: tuple[int, int], body_anchor: dict[str, int]) -> None:
		x = self.body_origin[0] + body_anchor["x"] - part_anchor[0]
		y = self.body_origin[1] + body_anchor["y"] - part_anchor[1]
		canvas.alpha_composite(part, (x, y))

	def redraw_canvas(self) -> None:
		self.title_label.configure(text=f"{self.animation_name} - {self.direction.get().title()}-facing animation")
		preview = self.composite_preview()
		display_size = self.canvas_size * self.zoom
		scaled_preview = preview.resize((display_size, display_size), Image.Resampling.NEAREST)
		self.preview_image = ImageTk.PhotoImage(scaled_preview)
		self.canvas.delete("all")
		self.canvas.configure(scrollregion=(0, 0, display_size, display_size))
		self.canvas.create_image(0, 0, anchor="nw", image=self.preview_image)

		if self.zoom >= 4:
			for x in range(0, display_size + 1, self.zoom):
				self.canvas.create_line(x, 0, x, display_size, fill="#ffffff", stipple="gray25")
			for y in range(0, display_size + 1, self.zoom):
				self.canvas.create_line(0, y, display_size, y, fill="#ffffff", stipple="gray25")

		for name, point in self.current_points.items():
			x = (self.body_origin[0] + point["x"] + 0.5) * self.zoom
			y = (self.body_origin[1] + point["y"] + 0.5) * self.zoom
			self.canvas.create_oval(x - 5, y - 5, x + 5, y + 5, fill="#f4d03f", outline="#111111", width=2)
			self.canvas.create_text(x + 9, y - 9, anchor="sw", text=name, fill="white")

		self.update_frame_controls()
		self.status_label.configure(text="Select an anchor and click or drag on the body. Scroll to zoom; right-drag to pan.")

	def update_frame_controls(self) -> None:
		is_new_frame = self.current_frame_index == len(self.frames)
		if is_new_frame:
			self.frame_indicator.configure(text=f"New Frame {len(self.frames) + 1} (Saved: {len(self.frames)})")
		else:
			self.frame_indicator.configure(text=f"Frame {self.current_frame_index + 1} of {len(self.frames)}")
		self.previous_button.configure(state="normal" if self.current_frame_index > 0 else "disabled")
		self.next_button.configure(state="normal" if self.current_frame_index < len(self.frames) else "disabled")

	def change_zoom(self, event: tk.Event[tk.Misc]) -> None:
		zoom_in = getattr(event, "delta", 0) > 0 or getattr(event, "num", 0) == 4
		new_zoom = min(MAX_ZOOM, self.zoom + 1) if zoom_in else max(1, self.zoom - 1)
		if new_zoom == self.zoom:
			return
		self.zoom = new_zoom
		self.redraw_canvas()

	def start_pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_mark(event.x, event.y)

	def pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_dragto(event.x, event.y, gain=1)

	def move_selected_point(self, event: tk.Event[tk.Misc]) -> None:
		x = int(self.canvas.canvasx(event.x) / self.zoom) - self.body_origin[0]
		y = int(self.canvas.canvasy(event.y) / self.zoom) - self.body_origin[1]
		x = max(0, min(x, self.body.width - 1))
		y = max(0, min(y, self.body.height - 1))
		point = self.current_points[self.selected_point.get()]
		point["x"] = x
		point["y"] = y
		point.setdefault("rotation", 0.0)
		self.redraw_canvas()

	def save_frame(self) -> None:
		if self.current_frame_index < len(self.frames):
			frame_name = self.frames[self.current_frame_index]["name"]
			self.frames[self.current_frame_index]["attachment_points"] = copy.deepcopy(self.current_points)
			messagebox.showinfo("Frame updated", f"Updated {frame_name}.")
		else:
			frame_name = f"frame_{len(self.frames) + 1}"
			self.frames.append({"name": frame_name, "attachment_points": copy.deepcopy(self.current_points)})
			self.current_frame_index += 1
			messagebox.showinfo("Frame saved", f"Saved {frame_name}.")
		self.redraw_canvas()

	def previous_frame(self) -> None:
		if self.current_frame_index == 0:
			return
		self.current_frame_index -= 1
		self.set_current_points(self.frames[self.current_frame_index]["attachment_points"])
		self.redraw_canvas()

	def next_frame(self) -> None:
		if self.current_frame_index >= len(self.frames):
			return
		self.current_frame_index += 1
		if self.current_frame_index < len(self.frames):
			self.set_current_points(self.frames[self.current_frame_index]["attachment_points"])
		else:
			self.set_current_points(self.frames[-1]["attachment_points"])
		self.redraw_canvas()

	def toggle_play(self) -> None:
		if self.is_playing:
			self.stop_play()
		else:
			self.start_play()

	def start_play(self) -> None:
		if not self.frames:
			messagebox.showwarning("No frames", "Save at least one frame before playing.")
			return
		self.is_playing = True
		self.play_button.configure(text="Pause")
		self.advance_playback_frame()

	def stop_play(self) -> None:
		self.is_playing = False
		self.play_button.configure(text="Play")
		if self.play_timer is not None:
			self.root.after_cancel(self.play_timer)
			self.play_timer = None

	def advance_playback_frame(self) -> None:
		if not self.is_playing or not self.frames:
			return
		if self.current_frame_index >= len(self.frames):
			self.current_frame_index = 0
		else:
			self.current_frame_index = (self.current_frame_index + 1) % len(self.frames)
		self.set_current_points(self.frames[self.current_frame_index]["attachment_points"])
		self.redraw_canvas()

		try:
			spf = max(0.05, float(self.spf_var.get()))
		except (ValueError, tk.TclError):
			spf = 1.0
		delay_ms = int(spf * 1000)
		self.play_timer = self.root.after(delay_ms, self.advance_playback_frame)

	def save_animation(self) -> None:
		if not self.frames:
			messagebox.showwarning("No frames", "Save at least one frame before saving the animation.")
			return
		ANIMATION_DIR.mkdir(exist_ok=True)
		animation = {
			"name": self.animation_name,
			"direction": self.direction.get(),
			"frames": self.frames,
		}
		output_path = ANIMATION_DIR / f"{self.animation_name}_{self.direction.get()}.json"
		output_path.write_text(json.dumps(animation, indent=2) + "\n", encoding="utf-8")
		messagebox.showinfo("Animation saved", f"Saved {output_path.relative_to(PROJECT_DIR)}")


def main() -> None:
	parser = argparse.ArgumentParser(description="Create a crab animation by posing a crab config's limbs.")
	parser.add_argument("--name", help="Animation name and output JSON filename.")
	parser.add_argument("--crab-config", help="Crab config name (from crab_configs/) to preview and pose.")
	args = parser.parse_args()

	root = tk.Tk()
	animation_name = args.name or simpledialog.askstring("Animation name", "Animation name (for example, walk):", parent=root)
	if not animation_name:
		root.destroy()
		return

	configs = crab_config_names()
	if not configs:
		messagebox.showerror("No crab configs", f"No crab config JSON files found in {CRAB_CONFIG_DIR}.")
		root.destroy()
		return
	crab_config_name = args.crab_config if args.crab_config in configs else configs[0]

	AnimationMapper(root, animation_name.strip(), crab_config_name)
	root.mainloop()


if __name__ == "__main__":
	main()
