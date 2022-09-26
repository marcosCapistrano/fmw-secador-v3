import { useEffect } from "react";
import { Outlet, Link } from "react-router-dom"
import paliniLogoBig from "./assets/logo-palini-big.png"
import "./index.css"

export default function Root() {
    useEffect(() => {
        document.body.classList.add('bg-gray-100');

        return () => {
            document.body.classList.remove('bg-gray-100');
        };
    }, []);

    return (
        <div className="p-8">
            <img src={paliniLogoBig} className="h-20"></img>
            <div className="my-8 p-4 bg-white rounded-lg">
                <div id="detail">
                    <Outlet />
                </div>
            </div>
        </div>
    );
}