import streamlit as st
import pandas as pd
import os
import glob

st.set_page_config(page_title="Collatz Research Dashboard", layout="wide")

st.title("🚀 Collatz Computational Mathematics Dashboard")
st.markdown("Analyze the CSV data exported by the C++ engine.")

data_dir = "data/csv"

if not os.path.exists(data_dir):
    st.warning("No data found. Please run the C++ executable to generate CSV exports first.")
    st.stop()

tab1, tab2, tab3 = st.tabs(["🌳 Reverse Tree", "📊 v_2 Distribution", "🔢 Residue Classes"])

with tab1:
    st.header("Reverse Collatz Tree Expansion")
    tree_files = glob.glob(f"{data_dir}/reverse_tree_*.csv")
    if tree_files:
        selected_file = st.selectbox("Select Experiment", tree_files, key="tree")
        df = pd.read_csv(selected_file)
        st.line_chart(df, x='depth', y='nodes')
        st.dataframe(df)
    else:
        st.info("Run: `./build/collatz.exe tree 50`")

with tab2:
    st.header("Probability of Diving by 2^k")
    stats_files = glob.glob(f"{data_dir}/statistical_v2_limit_*.csv")
    if stats_files:
        selected_file = st.selectbox("Select Experiment", stats_files, key="stats")
        df = pd.read_csv(selected_file)
        st.bar_chart(df, x='v_2', y=['measured_prob', 'theoretical_prob'])
        st.dataframe(df)
    else:
        st.info("Run: `./build/collatz.exe stats 10000000`")

with tab3:
    st.header("Residue Class Averages")
    residue_files = glob.glob(f"{data_dir}/residue_*.csv")
    if residue_files:
        selected_file = st.selectbox("Select Experiment", residue_files, key="residue")
        df = pd.read_csv(selected_file)
        st.bar_chart(df, x='residue', y='avg_stopping_time')
        st.dataframe(df)
    else:
        st.info("Run: `./build/collatz.exe residue 10000000 64`")
