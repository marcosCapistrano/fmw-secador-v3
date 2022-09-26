import { Link } from "react-router-dom";

const lotes = [1, 2, 3];

export default function AllLotes() {
    return (
        <ul className="items-center">
            {
                lotes.map(lote => (
                    <li>
                        <Link to={`/lote/${lote}`} >
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