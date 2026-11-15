#Note

## Formulation
Let P be inital path selection, d_P (s,u) is the distance following the inital path selection, d_(G-e) (u,t) is the recovery.
We want to minimize
$C(P,e)=d_P (s,u)+d_(G-e) (u,t)$

## Assumptions
- Undirected, connected, positive-weight graph; 
- \(k=1\); 
- blockage discovered at an endpoint; 
- an path \(s\)-to-\(t\) route remains available after blcokage.
- route and recovery is offline computed
## Literture
[A note on the k-Canadian Traveller Problem](https://pdf.sciencedirectassets.com/271527/1-s2.0-S0020019008X00067/1-s2.0-S0020019007002876/main.pdf?X-Amz-Security-Token=IQoJb3JpZ2luX2VjELf%2F%2F%2F%2F%2F%2F%2F%2F%2F%2FwEaCXVzLWVhc3QtMSJHMEUCIQDLp%2B94hVQiX8tDIJAW6Zs0DaYQUEI5Yt2oZRhwX90qQwIgGofdhcArOvmOUe15wU0%2F6p0hI9U5tzQcOHgFwie1BWQqswUIfxAFGgwwNTkwMDM1NDY4NjUiDLCycLSD1OO2o2cEXiqQBX53bLse%2FggUQDZlHClDL%2F7l7CjQiiH5KNtNnTvcrb%2Fn3vxHQCcCXGLQr5IUySsgc0PtvFKciclPJW%2Be%2BKBKvC6IGNxCv85Df2RGntsewyS05EC5zQW5z249OD3nvCNeGVZpLcpb07QKdl04tWiKvReL0%2FZjxPzOTPVZVa0OQtGQm336ht3gyXNAnQ39JYFt%2Btw6B3oGafccsa%2F4ewmQVsVSO1pBHR7snqt4rQ7yfaL%2FS9c3kLwlsKpACbTrlalAZEIcMrfKgLD15QTXwv%2BUJf9Q5ylWQ1HuNl8Pt0vqGRtzIm4XCF2iEN3%2BNGxptLm873Cf6R4hDWHPuY%2BH8OMOlOl4wGls4IDarMrp93BA8xCVOTMhNAjxyZwF6ca4cg6yV76udEhfwCXZDeii7QXG%2BL3HVMmmHgS%2FfqSZF9MAjcgpdfjreiNSAO%2BU8joZn20Hycqxp9cMOnuh6yoJ0Ti3vzDHTUaHFXM2DgXpjpUzq6UXx0n1TqvkXyG2VndT6VDk6tF5qIitQiMEtUa1gZuYFhcfe2R0qy3m70ee2CpVTPwVmiPYSRvpgFE4ZWueat2TBYkEXOHAGbxuspfiAl%2FErA%2B0HdP1VR550uMcz3H860IlB6dU2Q%2BqCXLuL%2FcRombNfK%2FhHxkOEs2U205LDpRE21GDWDqdictE1tZSop5SpROtzaeyO2qyJ9oJXWxB0YrNwjSKviP98rhkqe7Pz2TWaTxx7A4thEqrS9ukSSfYMjnj48U11rPrwyoZs5xnmOFwZ15wIFRe4fNLXEP627YTfArg%2FuKVcan0kIGArpXhjSUPVNDnEkDFfTE2Wcb5G7sQZTF%2FRKfrmNAssGKKNvr7Dd0JLPgXiLljhipuTwH76zG9MNK4otcGOrEBVpYF4swcVLYpHwsjAfVeTXw%2F%2FSLquB99uFOzhTfd9wyN%2B6IlevgRX6NGUnEgnEvFvDkgIpM31aodwbLtOc6HxTTl43vV%2BzcRy1bqit8gAG0g4Z14EWsE5q0rO63fd8Ta19MLkU9VD2MFTfGOGyjBG34zE6tGqaol9E0bcsxQKTIQ4DuxFmEW2LbR8WhjZ%2FHYa3YZsCLgEXYBdUvkdDEZLsUsiacF%2BBNvEngw%2BNaiU1Jn&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Date=20261102T150442Z&X-Amz-SignedHeaders=host&X-Amz-Expires=299&X-Amz-Credential=ASIAQ3PHCVTY7KVHG6JB%2F20261102%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Signature=7112a96c03b24da0fce265be47d727f55525d8815fbaa58d4ac6bd9b39de2b54&hash=3bdae5d8da46c02131d2d9f42149ca6d168154f7236f9113726cbff09f38e876&host=68042c943591013ac2b2430a89b270f6af2c76d8dfd086a07176afe7c76c2c61&pii=S0020019007002876&tid=spdf-0640827f-f825-4657-a2b3-60da01f52126&sid=fe3d6d2a7136764126991e14be1e1e09e3fcgxrqa&type=client&tsoh=d3d3LnNjaWVuY2VkaXJlY3QuY29t&rh=d3d3LnNjaWVuY2VkaXJlY3QuY29t&ua=1b13055255525e56570759&rr=a544087f7bb25c07&cc=au)

- For the original k-CTP, the lower bound of competative ratio is 2k+1.
    - traveller follow shortest path based on known info
    - meet blockage at the end of the path, backtrack to the starting point s. (giving 2*k)
    - after k th blockage, the remaining shortest path is the offline optimum (giving 1)
- However, full backtrack can be expensive

## Initial direction and Proposed algorithm
- Graph structure is important, if traveller planned path is s-u-t, when meet the blockage if there is an alternative path of u-t, it must form a cycle with the initial path.
- Greedy-style algorithm is limited, since alternative is discarded immediately, but we need to know recoveries to tell is a path better than other.