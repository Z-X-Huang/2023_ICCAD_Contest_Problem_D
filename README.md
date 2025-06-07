# 2023_ICCAD_Contest_Problem_D  
## 2023 ICCAD Contest - Problem D: Fixed-Outline Floorplanning with Rectilinear Soft Blocks

---

### 1. Introduction  
Modern VLSI designs rely on hierarchical flows and IP reuse to manage skyrocketing transistor counts. Early floorplanning not only estimates chip area but also predicts routing congestion and guides downstream optimizations. In this problem, you must pack deformable (“soft”) modules into a fixed chip outline while minimizing total wirelength, honoring both geometric and connectivity constraints.

---

### 2. Problem Statement  
**Given**  
- A fixed chip outline of dimensions **W × H**.  
- **N** soft modules, each with:  
  - A minimum area requirement  
  - An aspect-ratio range \[0.5, 2\]  
  - A utilization range \[80%, 100%\] (polygon area ÷ minimum enclosing rectangle area)  
- A set of two-pin nets connecting module pins.

**Find**  
- For each module, a **simple rectilinear polygon** (axis-aligned, no holes/self-intersections) and integer coordinates for its placement inside the outline, so that:  
  1. **No overlaps**; all modules lie entirely within the outline.  
  2. **Shape constraints** (area, aspect ratio, utilization) are met.  
  3. **Total HPWL** (half-perimeter wirelength) is minimized:  
     \[
       \text{Total HPWL} = \sum_{\text{all nets}} \bigl|x_i - x_j\bigr| + \bigl|y_i - y_j\bigr|
     \]

---

### 3. Evaluation

### 8.1 Scoring  
For each test case:
```text
score_case = (best_HPWL / your_HPWL)^2
es receive the same rank.  
