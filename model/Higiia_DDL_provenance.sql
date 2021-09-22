CREATE TABLE H_Provenance (
    tableName VARCHAR(120),
    userId INTEGER,
    imageId INTEGER,
    tStamp DATETIME, 
    operation VARCHAR(128),
    opValue TEXT,
    observation TEXT,
    PRIMARY KEY (tableName, userId, imageId, tStamp),
    FOREIGN KEY (userId, imageId, tableName) REFERENCES Pool (userId, imageId, tableName) ON DELETE CASCADE ON UPDATE CASCADE
);
    
    
