import sys
from pathlib import Path

# repo src directory is on sys.path
PROJECT_SRC = Path(__file__).resolve().parents[3]
if str(PROJECT_SRC) not in sys.path:
    sys.path.insert(0, str(PROJECT_SRC))

from benchmarking.python.plotting.LatencyPlot import LatencyPlot
from benchmarking.python.plotting.OrderingPlot import OrderingPlot
from benchmarking.python.plotting.utils import *
from benchmarking.python.plotting.ExchangePlot import ExchangePlot
from benchmarking.python.plotting.utils import *
from PIL import Image, ImageChops

class ImageEditor:

    def __init__(self, image_1: Path, image_2: Path):
        self.image_1 = Image.open(image_1).convert("RGBA")
        self.image_2 = Image.open(image_2).convert("RGBA")
        if self.image_2.size != self.image_1.size:
            self.image_2 = self.image_2.resize(image_1.size)

    def diff(self, output_name: str = "diff.png"):
        copy_1 = self.image_1
        copy_2 = self.image_2

        diff = ImageChops.difference(copy_1, copy_2)

        overlay = Image.alpha_composite(copy_1, copy_2)

        script_dir = Path(__file__).resolve().parent
        output_path = script_dir / output_name

        overlay.save(output_path)

        print(f"Saved diff to: {output_path}")

    # 128 is 50% transparency. 0 is fully transparent, 255 is opaque
    def transparency(self, alpha: int = 192, output_name: str = "overlay.png"):
        copy_2 = self.image_2
        copy_2.putalpha(alpha)

        copy_1 = self.image_1

        overlay = Image.alpha_composite(copy_1, copy_2)

        script_dir = Path(__file__).resolve().parent
        output_path = script_dir / output_name

        overlay.save(output_path)

        print(f"Saved overlay to: {output_path}")

if __name__ == "__main__":
    i1 = get_graph_dir("exchange") / "expected_matching_GOOD_matching_15_01_2026_10_32_38.png"
    i2 = get_graph_dir("exchange") / "actual_matching_GOOD_matching_15_01_2026_10_32_38.png"
    imgEditor = ImageEditor(i2, i1)
    imgEditor.diff()
    imgEditor.transparency()