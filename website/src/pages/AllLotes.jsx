import { useState, useEffect } from "react";
import { Link } from "react-router-dom";
import axios from 'axios';

const lotes = [1, 2, 3];

export default function AllLotes() {
    const [lotes, setLotes] = useState([]);

    useEffect(() => {
        async function fetchData() {
            const response = await axios.get(`/lotes`);
            let rawData;
            rawData = response.data;
            console.log(rawData);
            console.log(rawData.split(','));
            rawData = rawData.split(",").filter(a => a !== "");

            console.log(rawData);

            setLotes(rawData);
        }

        fetchData();
    }, []);

    return (
        <ul className="items-center">
            {
                lotes.map(lote => (
                    <li>
                        <Link to={`/historico/lote/${lote}`}>
                            <button className="bg-gray-200 m-2 p-4 w-fit rounded-lg hover:bg-blue-600 hover:text-white">
                                Lote {lote}
                            </button>
                        </Link>
                    </li>
                ))
            }
        </ul>
    )
}