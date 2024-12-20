# CHPS0703-Project :
Project of CHPS0703 which consists of an seam-carving algorithm implementation.

We chose to do it in C++ for better performance and because we didn't really like Python.

# Usage : 

To run this program, you need to first make the command:

````
make
````

Then you need to run the executable with the path of the image.
Ps: You don't need to worry about where the image will be saved, because the program does it automatically for you at the same location as the original image and creates a folder named "image".

Furthermore, specify the number of seams you want to take off the image.
Ps: 300 seams will take 300 rows or columns of pixels from the image depending on the next parameter.

Finally, specify 1 for column seams, 2 for row seams, and 3 for both.

This is an example for the image etretat.jpg, with 4,000 seams (as the image is 12,000 x 9,000) and 3 because we want both kinds of seams:

````
./main Images/etretat.jpg 4000 3
````

# Examples : 

Here are some examples of the algorithm on specified images.

<ins>Wikipedia's castle (with 500 seams):</ins>

![seamed_rows-chateau](https://github.com/user-attachments/assets/7a139aaf-f265-4b3b-b77a-bb386d74080e)
![resized_rows-chateau](https://github.com/user-attachments/assets/ec0b5b6f-bb7b-4cf1-a5a4-b83da9e355cf)
![seamed_cols-chateau](https://github.com/user-attachments/assets/23c42bf4-4d22-4487-8b8d-835e48c3f7cd)
![resized_cols-chateau](https://github.com/user-attachments/assets/56f5b048-e5d7-4d18-b96f-eb4ccfb8345d)

<ins>A "jungle" landscape (with 300 seams):</ins>

![seamed_rows-montagnes](https://github.com/user-attachments/assets/07dd3f20-faad-426c-8716-e5e3ef318f9c)
![resized_rows-montagnes](https://github.com/user-attachments/assets/a7508af9-f8f3-4a8c-9728-5769dafbe405)
![seamed_cols-montagnes](https://github.com/user-attachments/assets/ebdfba28-7358-4257-acf3-57dab7d43f6e)
![resized_cols-montagnes](https://github.com/user-attachments/assets/ba0c78e6-7bf6-400f-a32e-962ccee5fc68)

<ins>A bird (with 150 seams):</ins>

![seamed_rows-piaf](https://github.com/user-attachments/assets/6192e2d0-4a1c-4133-a411-ea37860889d0)
![resized_rows-piaf](https://github.com/user-attachments/assets/5a2a8cd3-3d12-4ab8-93ab-b70ba2583364)
![seamed_cols-piaf](https://github.com/user-attachments/assets/e27ba882-2e77-4682-9c64-428145a91b1f)
![resized_cols-piaf](https://github.com/user-attachments/assets/e57aea88-34ea-416c-90c0-e914702886aa)

<ins>A temple (with 200 seams):</ins>

![seamed_rows-temple](https://github.com/user-attachments/assets/2ac3f114-a9fc-4203-94d9-2709c7db7c7a)
![resized_rows-temple](https://github.com/user-attachments/assets/7496d422-241c-45a5-b9cd-a214a44b30a0)
![seamed_cols-temple](https://github.com/user-attachments/assets/c974c8f2-60b8-4e27-bffe-3ce95a350572)
![resized_cols-temple](https://github.com/user-attachments/assets/95f9e525-7c2d-4382-b14a-8b91118e2a99)

<ins>A picture of Etretat in France (with 4000 seams):</ins>
Ps: This is an image taken by myself in 12,000 x 9,000 resolution, which is too large for regular GitHub. That's why the images will be screenshots, as they are lighter.

![etretat_seamed_rows](https://github.com/user-attachments/assets/51a0f301-e044-4406-ab28-81da7a443ec4)
![etretat_resized_rows](https://github.com/user-attachments/assets/dd601873-d33e-43c6-8769-deff74f3b7b0)
![etretat_seamed_cols](https://github.com/user-attachments/assets/6ca34edc-d9f7-4637-9564-36e6b8b87196)
![etretat_resized_cols](https://github.com/user-attachments/assets/e2a3b2a2-4196-44bf-9ac8-7a4539df9101)
