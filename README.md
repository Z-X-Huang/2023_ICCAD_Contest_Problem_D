## 1 Introduction (Contest-Related Excerpts)

With the continuous advancement of technology, chip design complexity and transistor count are constantly increasing. [cite_start]Hierarchical design and IP (Intellectual Property) modules have been widely adopted to address this. [cite_start]Floorplanning plays a crucial role in these design methodologies, providing early feedback for system architecture evaluation and estimating chip area, as well as predicting interconnect-induced delays and congestion. [cite_start]Therefore, floorplanning remains an indispensable part of Very Large Scale Integration (VLSI) design and has become even more critical.

[cite_start]Beyond minimizing chip area, IC floorplanning often addresses other important challenges, such as fixed-outline constraints and soft modules. [cite_start]Solving these problems requires optimization methods and techniques to maximize design efficiency and quality.

[cite_start]Since chip dimensions are determined in the early design stages, leading to fixed-outline constraints, floorplanning methods that only consider area minimization are not entirely suitable for modern IC design. [cite_start]Fixed-outline floorplanning allows for proper placement of modules within the chip outline, maximizing space utilization and minimizing wire length. [cite_start]Therefore, fixed-outline floorplanning is essential in modern IC design.

[cite_start]Furthermore, fixed-outline floorplanning must be capable of handling soft modules (determining module shape and position) to ensure all modules can be legally placed within the fixed outline. [cite_start]Unlike hard modules (fixed width and height), soft modules can change their width and height without violating minimum area constraints, and can even have rectilinear (polygon) shapes.

## 2 Problem Statement

[cite_start]Given the width and height of the chip outline (with the bottom-left corner of the outline set as the origin (0,0)), a set of soft modules and their minimum required areas, a set of fixed-outline rectangular modules with their width, height, and bottom-left coordinates, and the number of connections between modules (including both soft and fixed-outline modules, all being two-pin nets), participants must implement a fixed-outline floorplanner. [cite_start]The objective is to minimize the total half-perimeter wirelength (total HPWL) by determining the shape and position of all soft modules, while adhering to all module shape and position constraints.

### 2.1 Module Shape and Position Constraints

There are four module shape constraints:

1.  [cite_start]The shape must be a simple rectilinear polygon.
    * [cite_start]Edges must be parallel to the axes of the rectangular coordinate system, and corners must be right angles.
    * [cite_start]All edges must be intersect-free.
    * [cite_start]The area enclosed by the polygon must be a single region with no holes.
2.  [cite_start]**Minimum Area Constraint**: Each soft module has a minimum area constraint, and the area enclosed by the module's shape must be greater than this limit.
3.  [cite_start]**Aspect Ratio Constraint**: The aspect ratio must be between 0.5 and 2. [cite_start]A soft module's aspect ratio is calculated from the smallest rectangle that can completely enclose it. [cite_start]The aspect ratio of a rectangle is its height divided by its width.
4.  [cite_start]**Rectangle Ratio Constraint**: The rectangle ratio must be between 80% and 100%. [cite_start]A soft module's rectangle ratio is calculated by dividing the area enclosed by its shape by the area of the smallest rectangle that can completely enclose it.

There are three module position constraints:

1.  [cite_start]The area enclosed by all module shapes must be entirely within the chip outline.
2.  [cite_start]Modules (including soft and fixed-outline modules) are not allowed to overlap.
3.  [cite_start]The corner coordinates of module shapes must be zero (0) or positive integers.

### 2.2 Legal and Illegal Module Shapes

[cite_start]For a soft module with a minimum area constraint of 20 units, aspect ratio constraint of 0.5 to 2, and rectangle ratio constraint of 80% to 100%, the following table lists 5 legal module shapes (not limited to these 5, grids are marked in the enclosed area for easy area calculation):

| Module Shape | Module Area | Aspect Ratio | Rectangle Ratio |
|---|---|---|---|
| (Image of shape) | [cite_start]20  | [cite_start]0.8 (=4/5)  | [cite_start]100% (=20/20)  |
| (Image of shape) | [cite_start]30  | [cite_start]0.83 (=5/6)  | [cite_start]100% (=30/30)  |
| (Image of shape) | [cite_start]20  | [cite_start]1.5 (=6/4)  | [cite_start]83.3% (=20/24)  |
| (Image of shape) | [cite_start]25  | [cite_start]1.2 (=6/5)  | [cite_start]83.3% (=25/30)  |
| (Image of shape) | [cite_start]27  | [cite_start]0.83 (=5/6)  | [cite_start]90% (=27/30)  |


[cite_start]The following table lists 5 illegal module shapes and reasons (not limited to these 5, grids are marked in the enclosed area for easy area calculation):

| Module Shape | Illegal Reason |
|---|---|
| (Image of shape) | [cite_start]Polygon enclosed area has a hole. |
| (Image of shape) | [cite_start]More than one polygon enclosed area. |
| (Image of shape) | [cite_start]Module area (18) is less than minimum area constraint (20). |
| (Image of shape) | [cite_start]Aspect ratio (10/2=5) violates constraint (0.5-2). |
| (Image of shape) | [cite_start]Rectangle ratio (20/36=55.6%) violates constraint (80%-100%). |


### 2.3 Calculation of Total Half-Perimeter Wirelength

[cite_start]The total half-perimeter wirelength for a floorplan is the sum of the Manhattan distances (center-to-center distance of the smallest enclosing rectangles [cite: 8][cite_start]) between each pair of modules multiplied by the number of connections between them.

## 8 Evaluation

1.  [cite_start]Participants' total scores will be ranked from highest to lowest.
2.  [cite_start]Each participant's total score is the sum of the scores independently calculated for each test case.
3.  [cite_start]For each test case, all legal floorplanning results generated by participants will be used, and the shortest total half-perimeter wirelength will be taken to calculate each participant's score according to Equation (1). [cite_start]The score will be calculated to three decimal places.

    $$\text{Score} = \left( \frac{\text{Shortest Total Half-Perimeter Wirelength}}{\text{This Participant's Total Half-Perimeter Wirelength}} \right)^2$$
   [cite_start][cite: 15]

4.  [cite_start]The floorplanning process and results generated by participants must comply with the following three points. [cite_start]If not, the floorplanning result will not be scored and will not be included in the valid floorplans generated by participants:
    * [cite_start]The program execution time (including file read/write processes) for each test case must be completed within 30 minutes.
    * [cite_start]The result for each test case must comply with the constraints described in "2.1 Module Shape and Position Constraints".
    * [cite_start]The result for each test case must comply with the file format described in "4. Output Format".
