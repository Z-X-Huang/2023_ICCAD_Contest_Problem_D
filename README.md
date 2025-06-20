# 2023 ICCAD Contest Problem D  
## 	Fixed-Outline Floorplanning with Rectilinear Soft Blocks  
---
### 1 Introduction  

Fixed-outline floorplanning must handle deformable modules (i.e., determining the shape and position of modules) to ensure all modules can be legally placed within the outline. Unlike hard modules (with fixed width and height), soft modules can alter their width and height within minimum area constraints and may even be shaped as polygons.  

---
### 2 Problem Statement
Implement a fixed-outline floorplanner. The goal is to minimize the total half-perimeter wirelength (HPWL) while satisfying all module shape and placement constraints by determining the shape and placement of all soft modules.

#### Module Shape and Placement Constraints
1. **Shape Constraints**: 
    * The shape must be a simple rectilinear polygon.  
    * Each soft module has a minimum area requirement; its shape must enclose an area greater than this limit.  
    * The aspect ratio (height/width) of the minimal bounding rectangle of a module must be within 0.5 to 2. For a rectangle, this is height divided by width.
    * The area of the module divided by the area of its minimal bounding rectangle must be between 80% and 100%.
2. **Placement Constraints**: 
    * All module shapes must be completely inside the chip outline.
    * Modules (both soft and hard) must not overlap.
    * The coordinates of polygon corners must be 0 or positive integers.

---
### 3 Evaluation  
For each testcase, the shortest legal HPWL among all contestants is used as a reference. The score for each testcase is calculated as:

$$\text{score} = \left( \frac{\text{shortest legal HPWL}}{\text{your HPWL}} \right)^2$$

---

| case1 | case02 | case03 |
|:---------------------------:|:---------------------------:|:---------------------------:|
| <img src="result/case1.jpg" width="500" height="325" />     | <img src="result/case2.jpg" width="500" height="325" />     | <img src="result/case3.jpg" width="500" height="325" />   |
