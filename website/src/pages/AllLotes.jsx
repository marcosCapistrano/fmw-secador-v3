import { useState, useEffect } from "react";
import { Link } from "react-router-dom";
import axios from 'axios';

const lotes = [1, 2, 3];

function SpaceAvailableBar() {
    const [space, setSpace] = useState({ used: 10, free: 90 });

    useEffect(() => {
        async function fetchData() {
            try {
                const response = await axios.get(`/space`);
                let rawData = response.data;

                console.log(rawData);
                console.log(rawData.split(','));
                rawData = rawData.split(",").filter(a => a !== "");
                console.log(rawData);

                let total = rawData[0];
                let used = rawData[1];

                setSpace({
                    used: (100 * used) / total,
                    free: 100 - ((100 * used) / total)
                });
            } catch (err) {
                console.log(err);
            }
        }

        fetchData();
    }, []);

    return (
        <div className="w-full  bg-red text-center items-center">
            <h1 className="w-full text-2xl pb-8">Memória do Dispositivo</h1>
            <div className="w-full flex items-center">
                <div className="w-full h-9 mx-auto border-gray-600 border-2 bg-gray-100 mb-2">
                    <div className={`h-full bg-red-500`} style={{ width: `${space.used.toString()}%` }}></div>
                </div>
            </div>

            <span className="block">{space.used}% Ocupado</span>
            <span className="block">{space.free}% Livre</span>
        </div>
    )

}

function Lote({ lote, forceUpdate }) {

    const handleOnClick = async ev => {
        const response = await axios.post(`/delete/${lote}`);

        if (response.status == 200) {
            forceUpdate();
        }
    }

    return (
        <li className="flex justify-around hover:border-2 p-2">
            <Link to={`/historico/lote/${lote}`}>
                <button className="bg-gray-200 m-2 p-4 w-fit rounded-lg hover:bg-blue-600 hover:text-white">
                    Lote {lote}
                </button>
            </Link>

            <button className="bg-yellow-300 m-2 p-4 w-fit rounded-lg hover:bg-blue-600 hover:text-white" onClick={handleOnClick}>Liberar Memória</button>
        </li>
    )
}

function Lotes() {
    const [lotes, setLotes] = useState([]);

    const loadData = async () => {
        const response = await axios.get(`/lotes`);
        let rawData;
        rawData = response.data;
        console.log(rawData);
        console.log(rawData.split(','));
        rawData = rawData.split(",").filter(a => a !== "");

        return rawData;
    }

    useEffect(() => {
        async function fetchData() {
            const response = await loadData();
            setLotes(response);
        }

        fetchData();
    }, []);

    return (
        <div className="pt-24 text-2xl text-center">
            <h1 className="pb-4">Lotes Disponíveis:</h1>
            <ul className="w-1/2 mx-auto">
                {
                    lotes.map(lote => (
                        <Lote lote={Number(lote)} forceUpdate={loadData} />
                    ))
                }
            </ul>
        </div>
    )
}

export default function AllLotes() {

    return (
        <>
            <SpaceAvailableBar />
            <Lotes />
        </>
    )
}