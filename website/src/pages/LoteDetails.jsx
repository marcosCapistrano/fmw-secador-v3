import { useState, useEffect } from "react";
import { csv, scaleBand, scaleLinear } from "d3";

const width = 700;
const height = 500;

const rawData = [
    1492389063, "SENSOR_ENTR", 25,
    1492389531, "SENSOR_ENTR", 24,
    1492389547, "SENSOR_ENTR", 25,
    1492389563, "SENSOR_ENTR", 24,
    1492390842, "SENSOR_ENTR", 25,
    1492393973, "SENSOR_ENTR", 26,
    1492393989, "SENSOR_ENTR", 28,
    1492394005, "SENSOR_ENTR", 29,
    1492394022, "SENSOR_ENTR", 30,
    1492394054, "SENSOR_ENTR", 29,
    1492394132, "SENSOR_ENTR", 28,
    1492394383, "SENSOR_ENTR", 26,
    1492394431, "SENSOR_ENTR", 32,
    1492394447, "SENSOR_ENTR", 33,
    1492394463, "SENSOR_ENTR", 32,
    1492394479, "SENSOR_ENTR", 31,
    1492394511, "SENSOR_ENTR", 30,
    1492394543, "SENSOR_ENTR", 29,
    1492394606, "SENSOR_ENTR", 28,
    1492394670, "SENSOR_ENTR", 27,
    1492394858, "SENSOR_ENTR", 26,
    1492394859, "SENSOR_ENTR", 27,
    1492394875, "SENSOR_ENTR", 26,
    1492394907, "SENSOR_ENTR", 28,
    1492394923, "SENSOR_ENTR", 30,
    1492394955, "SENSOR_ENTR", 29,
    1492395002, "SENSOR_ENTR", 28,
    1492395067, "SENSOR_ENTR", 30,
    1492395067, "SENSOR_ENTR", 31,
    1492395084, "SENSOR_ENTR", 33,
    1492395115, "SENSOR_ENTR", 32,
    1492395132, "SENSOR_ENTR", 31,
    1492395179, "SENSOR_ENTR", 33,
    1492395180, "SENSOR_ENTR", 34,
    1492395196, "SENSOR_ENTR", 35,
    1492395212, "SENSOR_ENTR", 34,
    1492395228, "SENSOR_ENTR", 33,
    1492395260, "SENSOR_ENTR", 32,
    1492395276, "SENSOR_ENTR", 31,
    1492395308, "SENSOR_ENTR", 30,
    775466466, "SENSOR_ENTR", 22,
    775466913, "SENSOR_ENTR", 25,
    775466930, "SENSOR_ENTR", 26,
    775466962, "SENSOR_ENTR", 27,
    775466995, "SENSOR_ENTR", 26,
    775467027, "SENSOR_ENTR", 25,
    775467105, "SENSOR_ENTR", 24,
    775467233, "SENSOR_ENTR", 23,
    775467334, "SENSOR_ENTR", 24,
    775467350, "SENSOR_ENTR", 25,
    775467366, "SENSOR_ENTR", 26,
    775467383, "SENSOR_ENTR", 28,
    775467431, "SENSOR_ENTR", 27,
    775467462, "SENSOR_ENTR", 26,
    775467514, "SENSOR_ENTR", 25,
    775467592, "SENSOR_ENTR", 24,
    775467776, "SENSOR_ENTR", 26,
    775467777, "SENSOR_ENTR", 27,
    775467793, "SENSOR_ENTR", 28,
    775467839, "SENSOR_ENTR", 27,
    775467886, "SENSOR_ENTR", 26,
    775467964, "SENSOR_ENTR", 25,
    775468068, "SENSOR_ENTR", 24,
    775468333, "SENSOR_ENTR", 23,
    775710152, "SENSOR_ENTR", 22,
    775710568, "SENSOR_ENTR", 21,
    775710600, "SENSOR_ENTR", 20,
    775711041, "SENSOR_ENTR", 21,
]

export default function LoteDetails() {
    const [data, setData] = useState(null);

    useEffect(() => {
        console.log(new Date(rawData[0]*1000))
        // csv(dataURL).then(data => {
        //     console.log("fetching");
        //     setData(data);
        //     console.log(data);
        // });
    }, []);

    if (!data) {
        return <pre>Loading...</pre>
    }

    const yScale = scaleBand()
        .domain(data.map(d => d.Estado))
        .range([0, height]);

    const xScale = scaleLinear()
        .domain([0, 80])
        .range([0, width]);

    return (
        <>
            <h1 className="text-center text-3xl pb-8">Lote 1</h1>

            <svg width={width} height={height}>
                {data.map((d, i) => <rect x={0} y={yScale(d.Estado)} width={xScale(d['Tx homicidios'])} height={yScale.bandwidth()} />)}
            </svg>
        </>
    )
}

                    // data.map((d, i) => (
                    //     <path fill={d.hex} d={pieArc({
                    //         startAngle: i / data.length * 2 * Math.PI,
                    //         endAngle: (i+1) / data.length * 2 * Math.PI
                    //     })} />
                    // ))