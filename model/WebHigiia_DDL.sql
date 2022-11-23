--Create metrics for existing distance functions (others can be added through Hermes library)
CREATE METRIC L2 
USING EUCLIDEAN 
FOR PARTICULATE;

CREATE METRIC L1 
USING CITYBLOCK 
FOR PARTICULATE;

CREATE METRIC CANBERRA 
USING CANBERRA  
FOR PARTICULATE;

-- Stores simple login for users -- This table will be replaced when QSsl starts supporting Web Assembly (client-side)
CREATE TABLE Login (
    id INTEGER,
    nick VARCHAR(45),
    pass VARCHAR(45),
    PRIMARY KEY (id)
);

-- Stores the pool of studies to be queried by a given user
CREATE TABLE Pool (
    userId INTEGER,
    imageId INTEGER,
    tableName VARCHAR (45),
    searchType INTEGER,
    PRIMARY KEY (userId, imageId, tableName)
);

-- Describe the target attributes of a given image dataset (including labels and attributes of interest)
CREATE TABLE Scope(
    tableName VARCHAR(45),
    attributeName VARCHAR(45),
    PRIMARY KEY (tableName, attributeName)
);

-- Provide a textual caption (short explanation) for the attributes included in the Scope
CREATE TABLE Caption (
    tableN VARCHAR (45),
    attributeN VARCHAR (45),
    valueN VARCHAR (45),
    caption VARCHAR (256),
    PRIMARY KEY (tableN, attributeN, valueN)
);
