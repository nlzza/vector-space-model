# Vector Space Model

This project was made in sixth semester for the course, "Information Retrieval". We were given a dataset of 30 files. The goal of this project was to form a vector space model.

This was accomplished by first implementing an inverted positional index. Stemming was done and stop words were excluded to reduce the size of the dictionary. Then each document was represented as a normalised vector in the feature space. The document vectors were then stored in a file.

When the user enters a query, the query is also represented as a normalised vector in the feature space. Then the document vectors are read from the file and for each document, the cosine similarity score between the document vector and the query vector is computed.

Only documents with score above a certain threshold are returned in result. The documents in result are ranked in descending order of score so that the most relevant document appears at the top.
