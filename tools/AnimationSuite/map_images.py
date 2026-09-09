"""Map crab-part image attachment points with a small Tkinter editor."""

from __future__ import annotations

import json
import shutil
import tkinter as tk
from pathlib import Path
from tkinter import messagebox, ttk


PROJECT_DIR = Path(__file__).resolve().parent
SOURCE_DIR = PROJECT_DIR / "source_imgs"
MAPPED_DIR = PROJECT_DIR / "mapped_imgs"
MAX_ZOOM = 128

BODY_ATTACHMENT_NAMES = (
	"left_front_leg",
	"left_middle_leg",
	"left_rear_leg",
	"right_front_leg",
	"right_middle_leg",
	"right_rear_leg",
	"left_claw",
	"right_claw",
)


class ImageMapper:
	"""Display each source PNG and collect its attachment coordinates."""

	def __init__(self, root: tk.Tk, image_paths: list[Path]) -> None:
		self.root = root
		self.image_paths = image_paths
		self.image_index = 0
		self.image: tk.PhotoImage | None = None
		self.display_image: tk.PhotoImage | None = None
		self.zoom = 1
		self.current_points: dict[str, dict[str, int]] = {}
		self.selected_point = tk.StringVar()

		root.title("Crab Image Mapper")
		root.resizable(True, True)
		root.minsize(720, 560)

		self.title_label = ttk.Label(root, font=("TkDefaultFont", 14, "bold"))
		self.title_label.pack(anchor="w", padx=12, pady=(12, 4))

		self.instructions_label = ttk.Label(root)
		self.instructions_label.pack(anchor="w", padx=12, pady=(0, 10))

		content = ttk.Frame(root, padding=(12, 0, 12, 12))
		content.pack(fill="both", expand=True)
		content.columnconfigure(1, weight=1)
		content.rowconfigure(0, weight=1)

		self.point_frame = ttk.LabelFrame(content, text="Attachment point", padding=8)
		self.point_frame.grid(row=0, column=0, sticky="nsw", padx=(0, 12))

		self.canvas = tk.Canvas(content, background="#303030", highlightthickness=0)
		self.canvas.grid(row=0, column=1, sticky="nsew")
		self.canvas.bind("<Button-1>", self.move_selected_point)
		self.canvas.bind("<B1-Motion>", self.move_selected_point)
		self.canvas.bind("<Button-3>", self.start_pan)
		self.canvas.bind("<B3-Motion>", self.pan)
		self.canvas.bind("<MouseWheel>", self.change_zoom)
		self.canvas.bind("<Button-4>", self.change_zoom)
		self.canvas.bind("<Button-5>", self.change_zoom)

		controls = ttk.Frame(root, padding=(12, 0, 12, 12))
		controls.pack(fill="x")
		self.save_button = ttk.Button(controls, text="Save and Continue", command=self.save_and_continue)
		self.save_button.pack(side="right")
		ttk.Button(controls, text="Skip", command=self.next_image).pack(side="right", padx=(0, 8))

		self.load_image()

	@property
	def current_path(self) -> Path:
		return self.image_paths[self.image_index]

	def attachment_names(self) -> tuple[str, ...]:
		if self.current_path.stem.startswith("body_"):
			return BODY_ATTACHMENT_NAMES
		return ("attachment_point",)

	def load_image(self) -> None:
		self.current_points = {}
		self.zoom = 1
		self.selected_point.set(self.attachment_names()[0])

		self.image = tk.PhotoImage(file=self.current_path)
		self.redraw_canvas()
		self.title_label.configure(
			text=f"{self.current_path.name} ({self.image_index + 1} of {len(self.image_paths)})"
		)
		self.instructions_label.configure(
			text="Select an anchor, then click or drag on the image to position it."
		)

		for child in self.point_frame.winfo_children():
			child.destroy()
		for name in self.attachment_names():
			ttk.Radiobutton(
				self.point_frame,
				text=name.replace("_", " ").title(),
				value=name,
				variable=self.selected_point,
			).pack(anchor="w", pady=2)

	def redraw_canvas(self) -> None:
		if self.image is None:
			return

		width = self.image.width() * self.zoom
		height = self.image.height() * self.zoom
		self.display_image = self.image.zoom(self.zoom, self.zoom)
		self.canvas.delete("all")
		self.canvas.configure(scrollregion=(0, 0, width, height))
		self.canvas.create_image(0, 0, anchor="nw", image=self.display_image)

		if self.zoom >= 4:
			for x in range(0, width + 1, self.zoom):
				self.canvas.create_line(x, 0, x, height, fill="#ffffff", stipple="gray25", tags="grid")
			for y in range(0, height + 1, self.zoom):
				self.canvas.create_line(0, y, width, y, fill="#ffffff", stipple="gray25", tags="grid")

		for name, point in self.current_points.items():
			self.draw_marker(name, point["x"], point["y"])

	def change_zoom(self, event: tk.Event[tk.Misc]) -> None:
		zoom_in = getattr(event, "delta", 0) > 0 or getattr(event, "num", 0) == 4
		new_zoom = min(MAX_ZOOM, self.zoom + 1) if zoom_in else max(1, self.zoom - 1)
		if new_zoom == self.zoom:
			return

		mouse_x = self.canvas.canvasx(event.x)
		mouse_y = self.canvas.canvasy(event.y)
		image_x = mouse_x / self.zoom
		image_y = mouse_y / self.zoom
		self.zoom = new_zoom
		self.redraw_canvas()
		self.canvas.xview_moveto(max(0, (image_x * self.zoom - event.x) / (self.image.width() * self.zoom)))
		self.canvas.yview_moveto(max(0, (image_y * self.zoom - event.y) / (self.image.height() * self.zoom)))

	def start_pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_mark(event.x, event.y)

	def pan(self, event: tk.Event[tk.Misc]) -> None:
		self.canvas.scan_dragto(event.x, event.y, gain=1)

	def move_selected_point(self, event: tk.Event[tk.Misc]) -> None:
		if self.image is None:
			return

		x = int(self.canvas.canvasx(event.x) / self.zoom)
		y = int(self.canvas.canvasy(event.y) / self.zoom)
		x = max(0, min(x, self.image.width() - 1))
		y = max(0, min(y, self.image.height() - 1))
		name = self.selected_point.get()
		self.current_points[name] = {"x": x, "y": y}
		self.draw_marker(name, x, y)

	def draw_marker(self, name: str, x: int, y: int) -> None:
		self.canvas.delete(f"marker-{name}")
		display_x = (x + 0.5) * self.zoom
		display_y = (y + 0.5) * self.zoom
		self.canvas.create_oval(
			display_x - 5,
			display_y - 5,
			display_x + 5,
			display_y + 5,
			fill="#f4d03f",
			outline="#111111",
			width=2,
			tags=f"marker-{name}",
		)
		self.canvas.create_text(
			display_x + 9,
			display_y - 9,
			anchor="sw",
			text=name,
			fill="white",
			tags=f"marker-{name}",
		)

	def save_and_continue(self) -> None:
		required_points = self.attachment_names()
		missing_points = [name for name in required_points if name not in self.current_points]
		if missing_points:
			messagebox.showwarning("Missing attachment points", "Set every attachment point before saving.")
			return

		if self.image is None:
			return

		MAPPED_DIR.mkdir(exist_ok=True)
		mapped_path = MAPPED_DIR / self.current_path.name
		shutil.copy2(self.current_path, mapped_path)

		config: dict[str, object] = {
			"image_path": mapped_path.relative_to(PROJECT_DIR).as_posix(),
			"source_image_size": {"width": self.image.width(), "height": self.image.height()},
		}
		if self.current_path.stem.startswith("body_"):
			config["attachment_points"] = self.current_points
		else:
			point = self.current_points["attachment_point"]
			config["attachment_point_x"] = point["x"]
			config["attachment_point_y"] = point["y"]

		config_path = MAPPED_DIR / f"{self.current_path.stem}.json"
		config_path.write_text(json.dumps(config, indent=2) + "\n", encoding="utf-8")
		self.next_image()

	def next_image(self) -> None:
		self.image_index += 1
		if self.image_index >= len(self.image_paths):
			messagebox.showinfo("Mapping complete", "All images have been processed.")
			self.root.destroy()
			return
		self.load_image()


def main() -> None:
	image_paths = sorted(SOURCE_DIR.glob("*.png"))
	if not image_paths:
		raise SystemExit(f"No PNG files found in {SOURCE_DIR}")

	root = tk.Tk()
	ImageMapper(root, image_paths)
	root.mainloop()


if __name__ == "__main__":
	main()
