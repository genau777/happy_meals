#!/usr/bin/env python3
"""
Script to populate HappyMeals database with recipes from TheMealDB API
"""

import sqlite3
import urllib.request
import json
import time

DB_PATH = "data/dishes.sqlite"
API_BASE = "https://www.themealdb.com/api/json/v1/1"

# Mapping TheMealDB categories to our dish types
CATEGORY_MAPPING = {
    "Breakfast": "Завтрак",
    "Dessert": "Десерт",
    "Starter": "Салат",
    "Side": "Салат",
    "Beef": "Второе",
    "Chicken": "Второе",
    "Lamb": "Второе",
    "Pork": "Второе",
    "Seafood": "Второе",
    "Pasta": "Второе",
    "Vegetarian": "Второе",
    "Vegan": "Второе",
    "Miscellaneous": "Второе"
}

# Mapping TheMealDB areas to our cuisines
CUISINE_MAPPING = {
    "American": "Американская",
    "British": "Британская",
    "Canadian": "Канадская",
    "Chinese": "Китайская",
    "Croatian": "Хорватская",
    "Dutch": "Голландская",
    "Egyptian": "Египетская",
    "French": "Французская",
    "Greek": "Греческая",
    "Indian": "Индийская",
    "Irish": "Ирландская",
    "Italian": "Итальянская",
    "Jamaican": "Ямайская",
    "Japanese": "Японская",
    "Kenyan": "Кенийская",
    "Malaysian": "Малайзийская",
    "Mexican": "Мексиканская",
    "Moroccan": "Марокканская",
    "Polish": "Польская",
    "Portuguese": "Португальская",
    "Russian": "Русская",
    "Spanish": "Испанская",
    "Thai": "Тайская",
    "Tunisian": "Тунисская",
    "Turkish": "Турецкая",
    "Vietnamese": "Вьетнамская",
    "Unknown": "Интернациональная"
}

def create_connection():
    """Create database connection and initialize tables"""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()
    
    # Create tables
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS dishes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name VARCHAR(100) NOT NULL,
            cuisine VARCHAR(50),
            dish_type VARCHAR(50),
            prep_time INTEGER,
            description TEXT,
            instructions TEXT,
            image_url TEXT
        )
    """)
    
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS ingredients (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name VARCHAR(100) NOT NULL UNIQUE,
            category VARCHAR(50)
        )
    """)
    
    cursor.execute("""
        CREATE TABLE IF NOT EXISTS dish_ingredients (
            dish_id INTEGER,
            ingredient_id INTEGER,
            measure VARCHAR(100),
            FOREIGN KEY(dish_id) REFERENCES dishes(id),
            FOREIGN KEY(ingredient_id) REFERENCES ingredients(id),
            PRIMARY KEY(dish_id, ingredient_id)
        )
    """)
    
    conn.commit()
    return conn

def fetch_json(url):
    """Fetch JSON from URL"""
    with urllib.request.urlopen(url) as response:
        return json.loads(response.read().decode())

def fetch_all_meals():
    """Fetch meals from TheMealDB API"""
    print("Fetching meals from TheMealDB API...")
    
    all_meals = []
    
    # Fetch 50 random meals (faster approach)
    print("Fetching 50 random meals...")
    for i in range(50):
        try:
            random_url = f"{API_BASE}/random.php"
            data = fetch_json(random_url)
            meal = data.get('meals', [])
            
            if meal:
                all_meals.append(meal[0])
                print(f"  {i+1}/50 - {meal[0]['strMeal']}")
            
            time.sleep(0.05)  # Small delay to be nice to API
        except Exception as e:
            print(f"  Error fetching meal {i+1}: {e}")
            continue
    
    print(f"\nTotal meals fetched: {len(all_meals)}")
    return all_meals

def insert_ingredient(conn, ingredient_name):
    """Insert ingredient and return its ID"""
    cursor = conn.cursor()
    
    # Check if ingredient exists
    cursor.execute("SELECT id FROM ingredients WHERE name = ?", (ingredient_name,))
    result = cursor.fetchone()
    
    if result:
        return result[0]
    
    # Insert new ingredient
    cursor.execute("INSERT INTO ingredients (name, category) VALUES (?, ?)", 
                   (ingredient_name, "Общее"))
    conn.commit()
    return cursor.lastrowid

def estimate_prep_time(instructions):
    """Estimate preparation time based on instructions length"""
    if not instructions:
        return 30
    
    word_count = len(instructions.split())
    
    if word_count < 100:
        return 15
    elif word_count < 200:
        return 30
    elif word_count < 400:
        return 45
    else:
        return 60

def populate_database(meals):
    """Populate database with meals data"""
    conn = create_connection()
    cursor = conn.cursor()
    
    print("\nPopulating database...")
    
    for meal in meals:
        name = meal['strMeal']
        cuisine = CUISINE_MAPPING.get(meal.get('strArea', 'Unknown'), 'Интернациональная')
        dish_type = CATEGORY_MAPPING.get(meal.get('strCategory', 'Miscellaneous'), 'Второе')
        instructions = meal.get('strInstructions', '')
        prep_time = estimate_prep_time(instructions)
        description = meal.get('strInstructions', '')[:200] + "..." if meal.get('strInstructions') else ""
        image_url = meal.get('strMealThumb', '')
        
        # Insert dish
        cursor.execute("""
            INSERT INTO dishes (name, cuisine, dish_type, prep_time, description, instructions, image_url)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        """, (name, cuisine, dish_type, prep_time, description, instructions, image_url))
        
        dish_id = cursor.lastrowid
        
        # Insert ingredients
        for i in range(1, 21):  # TheMealDB has up to 20 ingredients
            ingredient_key = f'strIngredient{i}'
            measure_key = f'strMeasure{i}'
            
            ingredient_name = meal.get(ingredient_key, '').strip()
            measure = meal.get(measure_key, '').strip()
            
            if ingredient_name:
                ingredient_id = insert_ingredient(conn, ingredient_name)
                
                # Link dish and ingredient (skip if already exists)
                try:
                    cursor.execute("""
                        INSERT INTO dish_ingredients (dish_id, ingredient_id, measure)
                        VALUES (?, ?, ?)
                    """, (dish_id, ingredient_id, measure))
                except sqlite3.IntegrityError:
                    # Ingredient already linked to this dish, skip
                    pass
        
        conn.commit()
        print(f"Added: {name}")
    
    conn.close()
    print("\nDatabase population complete!")

def main():
    print("=" * 60)
    print("HappyMeals Recipe Database Populator")
    print("Using TheMealDB API")
    print("=" * 60)
    
    # Fetch meals from API
    meals = fetch_all_meals()
    
    if not meals:
        print("No meals fetched. Exiting.")
        return
    
    # Populate database
    populate_database(meals)
    
    print("\n" + "=" * 60)
    print("Done! Your database now has real recipes with:")
    print("- Dish names and descriptions")
    print("- Ingredients with measurements")
    print("- Cooking instructions")
    print("- Cuisine types and categories")
    print("=" * 60)

if __name__ == "__main__":
    main()
