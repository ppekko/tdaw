# Building  
This demo plays the default sine ping code found on new shadertoy projects.

This has been structured so you can compare sizes between different compilers and backends, with or without compression.
All you need to do is go into no-multithread/ or multithread/, and run the `compileall.sh` script.

As for the Makefile hell, it works, and I don't want to touch it again. Just run the scripts

## Important!
As said in the main readme, these sizes may vary on your machine, and additionally ***if you incorporate them into your project, chances are it will be smaller due to compiler optimisations and better packing compression with larger files***

## Sizes


Compiled on Arch Linux with:
 - tcc (0.9.28rc)
 - gcc (13.2.1)
 - clang (16.0.6)

<details>
<summary>No Multithreading</summary>
<blockquote>
<details>
<summary>ALSA Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 1180 bytes | 5680 bytes   |
| GCC      | 1749 bytes | 12448 bytes  |
| CLANG    | 1722 bytes | 12448 bytes  |

</details>
</blockquote>
<blockquote>
<details>
<summary>PortAudio Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 1570 bytes | 4056 bytes   |
| GCC      | 1404 bytes | 12368 bytes  |
| CLANG    | 1384 bytes | 12368 bytes  |

</details>
</blockquote>
<blockquote>
<details>
<summary>aplay Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 1416 bytes | 3528 bytes   |
| GCC      | 1300 bytes | 12336 bytes  |
| CLANG    | 1278 bytes | 12336 bytes  |

</details>
</blockquote>
<br>
</details>
<!-------------------------------------------------->
<details>
<summary>Multithreading</summary>
<blockquote>
<details>
<summary>ALSA Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 2062 bytes | 6312 bytes   |
| GCC      | 1960 bytes | 12472 bytes  |
| CLANG    | 1931 bytes | 12472 bytes  |

</details>
</blockquote>
<blockquote>
<details>
<summary>PortAudio Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 1769 bytes | 4720 bytes   |
| GCC      | 1674 bytes | 12400 bytes  |
| CLANG    | 1655 bytes | 12400 bytes  |

</details>
</blockquote>
<blockquote>
<details>
<summary>aplay Backend</summary>

| Compiler  | Compressed  | Un-compresed  |
|----------|------------|--------------|
| TCC      | 1617 bytes | 4160 bytes   |
| GCC      | 1578 bytes | 12368 bytes  |
| CLANG    | 1553 bytes | 12368 bytes  |

</details>
</blockquote>
<br>
</details>
