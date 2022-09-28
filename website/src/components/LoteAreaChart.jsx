import {
    Chart as ChartJS,
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Filler,
    Legend,
} from 'chart.js';
import { Line, getDatasetAtEvent } from 'react-chartjs-2';
import { useState, useEffect, useRef } from 'react';


ChartJS.register(
    CategoryScale,
    LinearScale,
    PointElement,
    LineElement,
    Title,
    Tooltip,
    Filler,
    Legend
);



function extrapolateData(dataArray, labels) {
    let newDataArray = [];
    let lastValue = 0;
    for (let label of labels) {
        for (let entry of dataArray) {
            if (entry.date == label) {
                lastValue = entry.value;
                break;
            }
        }

        newDataArray.push(lastValue);
    }

    return newDataArray;
}

function createDataset(sublote, chart) {
    return {
        labels: sublote.labels.map(entry => new Date(entry * 1000).toLocaleString('pt-BR')),
        datasets: [
            {
                label: 'Entrada',
                data: extrapolateData(sublote.sensor_entr, sublote.labels),
                borderColor: 'rgba(0, 0, 128, 0.5)',
                backgroundColor: 'rgba(0, 0, 128, 0.1)',
            },
            {
                label: 'Massa 1',
                data: extrapolateData(sublote.sensor_m1, sublote.labels),
                borderColor: 'rgba(0, 128, 128, 0.5)',
                backgroundColor: 'rgba(0, 128, 128, 0.1)',
            },
            {
                label: 'Massa 2',
                data: extrapolateData(sublote.sensor_m2, sublote.labels),
                borderColor: 'rgba(170, 110, 40, 0.5)',
                backgroundColor: 'rgba(170, 110, 40, 1)',
            },
            {
                label: 'Massa 3',
                data: extrapolateData(sublote.sensor_m3, sublote.labels),
                borderColor: 'rgba(245, 130, 48, 0.5)',
                backgroundColor: 'rgba(245, 130, 48, 1)',
            },
            {
                label: 'Massa 4',
                data: extrapolateData(sublote.sensor_m4, sublote.labels),
                borderColor: 'rgba(220, 190, 255, 0.5)',
                backgroundColor: 'rgba(220, 190, 255, 1)',
            },
        ]
    }
}

export default function LoteAreaChart({ sublote }) {
    const [data, setData] = useState({ datasets: [] });
    const [options, setOptions] = useState({
        responsive: true,
        plugins: {
            legend: {
                position: 'top',
            },
            title: {
                display: false,
                text: "",
            },
        },
    });

    const chartRef = useRef(null);

    useEffect(() => {
        const chart = chartRef.current;

        if (!chart)
            return;

        console.log(createDataset(sublote, chart));
        // setOptions({...options, plugins: { ...options.plugins, title: { display: true, text: "oi"}}});
        setData(createDataset(sublote, chart));
    }, []);

    return (
        <div className='w-2/3 h-full mx-auto py-12 text-center'>
            <h1 className='text-2xl font-bold text-gray-800'>Ligado em: {new Date(sublote.initDate * 1000).toLocaleString('pt-BR')}</h1>
            {sublote.endDate ? <h1 className='text-lg text-gray-700 pb-8'>Desligado em: {new Date(sublote.endDate * 1000).toLocaleString('pt-BR')}</h1> : null}
            <Line ref={chartRef} options={options} data={data} />
        </div>
    );
}

function createGradientStroke(ctx, area) {
    const colorStart = "rgba(0, 0, 255, 0.5)";
    const colorMid = "rgba(0, 255, 0, 0.5)";
    const colorEnd = "rgba(255, 0, 0, 0.5)";

    const gradient = ctx.createLinearGradient(0, area.bottom, 0, area.top);

    gradient.addColorStop(0, colorStart);
    gradient.addColorStop(0.5, colorMid);
    gradient.addColorStop(1, colorEnd);

    return gradient;
}

function createGradientBackground(ctx, area) {
    const colorStart = "rgba(0, 0, 255, 0.2)";
    const colorMid = "rgba(0, 255, 0, 0.2)";
    const colorEnd = "rgba(255, 0, 0, 0.2)";

    const gradient = ctx.createLinearGradient(0, area.bottom, 0, area.top);

    gradient.addColorStop(0, colorStart);
    gradient.addColorStop(0.5, colorMid);
    gradient.addColorStop(1, colorEnd);

    return gradient;
}