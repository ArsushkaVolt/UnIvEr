// server/routes/cart.js
import express from 'express';
import db from '../db.js';

const router = express.Router();

// Добавить
router.post('/', async (req, res) => {
    const { user_id, product_name, price, quantity = 1 } = req.body;
    try {
        const [result] = await db.execute(
            'INSERT INTO cart (user_id, product_name, price, quantity) VALUES (?, ?, ?, ?)',
            [user_id, product_name, price, quantity]
        );
        res.json({ id: result.insertId });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Получить
router.get('/:user_id', async (req, res) => {
    const { user_id } = req.params;
    try {
        const [rows] = await db.execute('SELECT * FROM cart WHERE user_id = ?', [user_id]);
        res.json(rows);
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

// Удалить
router.delete('/:id', async (req, res) => {
    const { id } = req.params;
    try {
        const [result] = await db.execute('DELETE FROM cart WHERE id = ?', [id]);
        res.json({ deleted: result.affectedRows });
    } catch (err) {
        res.status(500).json({ error: err.message });
    }
});

export default router;