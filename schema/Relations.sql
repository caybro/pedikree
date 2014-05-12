CREATE TABLE Relations (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    type TEXT NOT NULL, -- FIXME maybe should be an enum like, so INT
    person1_id INTEGER REFERENCES People(id),
    person2_id INTEGER REFERENCES People(id),
    place_id INTEGER DEFAULT 0 REFERENCES Places(id) ON UPDATE SET NULL,
    date DATETIME,
    comment TEXT
);