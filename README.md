# 2023_ICCAD_Contest_Problem_D  
### 2023 ICCAD Contest - Problem D: Fixed-Outline Floorplanning with Rectilinear Soft Blocks

## 1. Introduction  
You are given a fixed rectangular chip outline, a mix of hard (fixed‐size) and soft (area‐flexible) modules, and a set of two‐pin connections. Your task is to choose shapes for the soft blocks and place all modules inside the outline to minimize total half‐perimeter wirelength (HPWL).

## 2. Problem Statement  
- **Chip**: Rectangle of width _W_ and height _H_.  
- **Fixed Modules** (_F_): Each has name, position _(x,y)_, width _w_, height _h_.  
- **Soft Modules** (_S_): Each has name and minimum area; shape must be a simple rectilinear polygon satisfying:  
  - Area ≥ minimum  
  - Aspect‐ratio between 0.5 and 2.0  
  - “Rectangular‐ratio” (area / bounding‐box‐area) between 0.8 and 1.0  
- **Connections** (_C_): Each two‐pin net connects exactly two modules.  

**Goal**:  
1. Select a legal polygonal shape for each soft module.  
2. Assign integer-coordinate, non-overlapping placements for all modules wholly inside the chip.  
### 3. Minimize total HPWL

**HPWL** = ∑₍ᵢ,ⱼ₎∈nets (│xᵢ − xⱼ│ + │yᵢ − yⱼ│)

---

### 8. Evaluation

- **Scoring**  
  For each test case:  
  **score_case** = (best HPWL / your HPWL)²  

  Total score is the sum over all cases; higher is better.
