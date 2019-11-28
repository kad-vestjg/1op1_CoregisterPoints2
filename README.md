# 1op1_CoregisterPoints2
Find corresepondings points within a certain range, using RTree (Boost)

## COREGISTERPOINTS (2)
--
 Performs coregistration of points within ONE datafile. Looks through all permutation of points
 to find those that are possibly the same, within a given range (10cm default).
 Note: On large datasets (what it is actually for), do not run it in debug mode (too slow)
 
--
 ## USAGE
 **Coregisterpoints2 <FileName.txt> [Distance (m)]**
 
 Option | Necessity | explanation
 ---|---|---
 <FileName.ext> | *(Required)* | Input filename of data, stored as Id Pn X Y Z in fixed format (!) Output in FileName_Output.csv and FileName_Output_Overview.csv
 [Distance (m)] | *(Optional)* | Range within which points are set to identical
 
--
## Formatting

### Dataformat of input file: (:: ClassicReadingRoutine = true)
 
` 1        10        20        30        40        50        60        70        80`
` +--------+---------+---------+---------+---------+---------+---------+---------+------->`
`       1417               10861       1779.559       2145.712          0.000    Comment`
`       1418               10862       1782.001       2146.667          0.000    Path`
`        ...                 ...            ...            ...            ...    .......`

 (Fortran format: '(I10,A20,3F15.3,A)' )
 
 
### Alternative dataformat of input file:  (:: ClassicReadingRoutine = false)
 
#### 4 arguments
 Point X Y Z
 1417 1779.559 2145.712 0.000
 1418 1782.001 2146.667 0.000
 .... ........ ........ .....
 

#### 5 arguments
 Id Point X Y Z
 0 1417 1779.559 2145.712 0.000
 1 1418 1782.001 2146.667 0.000
 . .... ........ ........ .....


### >5 arguments
 Id Point X Y Z Extra Unused Data
 0 1417 1779.559 2145.712 0.000 path comment bs ....
 1 1418 1782.001 2146.667 0.000 only_path ....
 . .... ........ ........ ..... ....... ....... ....
 Free format, separated by one or more spaces.
 
--

## External references/libraries:
**Boost (geometry/predicates/rtree)**
 
--
## Revision history

| Version | Author |           Date |         Changes |
| ------- | ------ |           ---- |         ------- |
| 1.0     | G.M.B. Vestjens  | 17-11-2017   | Finished version using a default distance of 10cm |
| 1.1     | G.M.B. Vestjens  | 09-03-2018   | Added a distance/range option for the commandline |

