# 2023_ICCAD_Contest_Problem_D  
## 2023 ICCAD Contest - Problem D: Fixed-Outline Floorplanning with Rectilinear Soft Blocks


### 1. Introduction  
As process technology and transistor density continue to increase, the scale and complexity of modern SoC designs have grown dramatically. Hierarchical design and silicon IP reuse are now standard practice, and early floorplanning plays a critical role in estimating chip area, predicting congestion, and guiding downstream optimization. In today’s VLSI flows, it is not enough to merely minimize area—you must also respect a fixed chip outline and accommodate deformable “soft” modules to achieve high-quality Power, Performance, and Area (PPA) results.

---

### 2. Problem Statement  
**Goal**: Given a fixed chip outline (width × height), choose both the shape (a simple rectilinear polygon) and position of each soft module so as to minimize the total Half-Perimeter Wirelength (HPWL), subject to:  

1. **Module Shape Constraints**  
   - Must be a **simple** rectilinear polygon (no self-intersections or holes; edges axis-aligned).  
   - Area ≥ module’s specified minimum.  
   - Aspect ratio (height/width) ∈ [0.5, 2].  
   - Utilization (polygon area ÷ its minimum enclosing rectangle area) ∈ [80%, 100%].  

2. **Module Placement Constraints**  
   - All modules lie entirely within the fixed outline and do not overlap.  
   - All polygon vertex coordinates are non-negative integers.  

3. **Nets and Objective**  
   - Each net connects exactly two pins (two-pin nets).  
   - **Total HPWL** = ∑<sub>all nets</sub> (Manhattan distance between pin centers).

---

### 3. Evaluation  
Submissions are ranked by **total score**, computed as the sum over all test cases of:

```txt
score_case = (best_HPWL / your_HPWL)^2
- **best_HPWL** is the shortest legal HPWL achieved by any team on that case.  
- Scores are calculated to three decimal places.

**Validity checks** for each test case:  
- Total runtime (including I/O) ≤ 30 minutes.  
- Final packing must satisfy all shape and placement constraints.  
- Output must conform exactly to the prescribed format.

Teams are then sorted by descending total score; ties receive the same rank.  
